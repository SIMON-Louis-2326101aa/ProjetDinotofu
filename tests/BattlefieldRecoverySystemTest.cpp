#include <cassert>

#include "class_system/ClassCatalog.hpp"
#include "core/Random.hpp"
#include "entity/Player.hpp"
#include "item/weapon/Weapon.hpp"

int main()
{
    Player player("Récupérateur", ClassCatalog::createClassByName("Guerrier"));
    player.getInventory().clearAll();

    Inventory cacheInventory;
    Weapon relic(
        "Relique du terrain",
        "Objet exact laissé après un KO.",
        1400,
        WeaponType::Sword,
        10,
        16,
        4,
        150);
    relic.setPersistentId("battlefield_recovery_exact_weapon_001");
    relic.loseDurability(23);
    relic.addEnchantment("Foudre contenue");
    relic.setPersonalRecoveryItem(true);
    relic.setUniqueRecoveryItem(true);
    relic.setRecoveryPriority(100);
    cacheInventory.addWeapon(relic);

    const std::string cacheId = player.createBattlefieldRecoveryCache(
        "postko_test", "Passage effondré", cacheInventory);
    assert(!cacheId.empty());
    assert(player.hasOpenBattlefieldRecoveryCacheForCity("valebrume"));
    assert(player.getInventory().getWeapons().empty());

    const int beforeTime = player.getWorldDayProgressUnits();
    Random random(7u);
    const BattlefieldRecoveryAttemptResult result = player.attemptBattlefieldRecovery(cacheId, random);
    assert(result.available);
    assert(result.success); // Same-day recovery is intentionally guaranteed.
    assert(result.recoveredProperty);
    assert(result.chancePercent == 100);
    assert(player.getWorldDayProgressUnits() != beforeTime || player.getWorldDaysElapsed() > 0);
    assert(!player.hasOpenBattlefieldRecoveryCacheForCity("valebrume"));
    assert(player.getBattlefieldRecoveryCaches().size() == 1);
    assert(player.getBattlefieldRecoveryCaches().front().resolved);
    assert(player.getBattlefieldRecoveryCaches().front().inventory.getWeapons().empty());
    assert(player.getBattlefieldRecoveryCaches().front().inventory.getArmors().empty());
    assert(player.getBattlefieldRecoveryCaches().front().inventory.getMaterials().empty());
    assert(player.getBattlefieldRecoveryCaches().front().inventory.getConsumables().empty());
    assert(player.getBattlefieldRecoveryCaches().front().inventory.getTotalCopper() == 0);

    assert(player.getInventory().getWeapons().size() == 1);
    const Weapon& recovered = player.getInventory().getWeapons().front();
    assert(recovered.getPersistentId() == relic.getPersistentId());
    assert(recovered.getDurability() == relic.getDurability());
    assert(recovered.getEnchantments() == relic.getEnchantments());
    assert(recovered.isPersonalRecoveryItem());
    assert(recovered.isUniqueRecoveryItem());
    assert(recovered.getRecoveryPriority() == 100);

    // A resolved cache cannot duplicate the same exact object.
    Random duplicateRandom(8u);
    const BattlefieldRecoveryAttemptResult duplicate = player.attemptBattlefieldRecovery(cacheId, duplicateRandom);
    assert(!duplicate.available);
    assert(player.getInventory().getWeapons().size() == 1);

    // Older unresolved loot can physically move through the world instead of staying at one magic coordinate.
    Player moving("Pisteur", ClassCatalog::createClassByName("Guerrier"));
    moving.getInventory().clearAll();
    Inventory movingInventory;
    Weapon movedRelic = relic;
    movedRelic.setPersistentId("battlefield_recovery_moving_weapon_001");
    movingInventory.addWeapon(movedRelic);
    const std::string movingId = moving.createBattlefieldRecoveryCache("postko_move", "Chemin des pierres", movingInventory);
    moving.advanceWorldDays(8);

    auto findFailSeed = [](int chance, bool wantGuardRoll) {
        for (unsigned int seed = 1; seed < 10000; ++seed)
        {
            Random probe(seed);
            const int first = probe.between(1, 100);
            if (first <= chance) continue;
            if (!wantGuardRoll || probe.between(1, 100) <= 25) return seed;
        }
        return 0u;
    };

    unsigned int seed1 = findFailSeed(60, false);
    assert(seed1 != 0);
    Random fail1(seed1);
    const auto firstMiss = moving.attemptBattlefieldRecovery(movingId, fail1);
    assert(firstMiss.available && !firstMiss.success);
    assert(moving.getBattlefieldRecoveryCaches().back().custodyState == "field");

    moving.advanceWorldDays(1);
    unsigned int seed2 = findFailSeed(59, false);
    assert(seed2 != 0);
    Random fail2(seed2);
    const auto secondMiss = moving.attemptBattlefieldRecovery(movingId, fail2);
    assert(!secondMiss.success);
    assert(secondMiss.custodyChanged);
    assert(moving.getBattlefieldRecoveryCaches().back().custodyState == "nearby_cache");
    assert(moving.getBattlefieldRecoveryCaches().back().movementCount == 1);

    moving.advanceWorldDays(1);
    // nearby_cache at this point: base 60 + attempts 14 - disturbance 16 - state 5 = 53.
    unsigned int seed3 = findFailSeed(53, true);
    assert(seed3 != 0);
    Random fail3(seed3);
    const auto thirdMiss = moving.attemptBattlefieldRecovery(movingId, fail3);
    assert(!thirdMiss.success);
    assert(thirdMiss.custodyChanged);
    assert(thirdMiss.securedByGuard);
    assert(moving.getBattlefieldRecoveryCaches().back().custodyState == "guard");
    assert(moving.getBattlefieldRecoveryCaches().back().movementCount == 2);

    moving.advanceWorldDays(1);
    Random guardRecovery(999u);
    const auto guarded = moving.attemptBattlefieldRecovery(movingId, guardRecovery);
    assert(guarded.success);
    assert(guarded.securedByGuard);
    assert(guarded.chancePercent == 100);
    assert(moving.getInventory().getWeapons().size() == 1);
    assert(moving.getInventory().getWeapons().front().getPersistentId() == "battlefield_recovery_moving_weapon_001");

    return 0;
}
