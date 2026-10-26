#include <cassert>

#include "class_system/ClassCatalog.hpp"
#include "economy/Money.hpp"
#include "entity/Player.hpp"
#include "item/inventory/CarryLoadSystem.hpp"
#include "item/material/Material.hpp"

namespace
{
    Material normalLoadMaterial(int quantity)
    {
        return Material(
            "load_test_bundle",
            "Lot de test",
            "Pile neutre utilisée pour régler les seuils d'encombrement.",
            "Composant",
            1,
            quantity
        );
    }
}

int main()
{
    Player player;
    player.setAppearanceProfile(18, "Test", "Morphologie standard");
    assert(CarryLoadSystem::backpackBaseCapacity(0) == 40);
    assert(CarryLoadSystem::backpackBaseCapacity(1) == 50);
    assert(CarryLoadSystem::backpackBaseCapacity(2) == 60);
    assert(CarryLoadSystem::backpackBaseCapacity(3) == 70);
    assert(CarryLoadSystem::backpackBaseCapacity(4) == 80);

    const CarryLoadSummary empty = CarryLoadSystem::evaluate(player);
    assert(empty.capacity >= 40);
    assert(empty.used == 0);
    assert(empty.escapeModifierPercent == 0);
    assert(empty.meleeAccuracyModifierPercent == 0);
    assert(empty.physicalProtectionPercent == 0);
    assert(!empty.movementBlocked);

    Weapon heavyWeapon(
        "Marteau massif de réserve",
        "Une arme lourde et encombrante gardée dans le sac pour le test.",
        100,
        WeaponType::Hammer,
        4,
        8,
        0,
        100
    );
    player.getInventory().addWeapon(heavyWeapon);
    const int withStoredWeapon = CarryLoadSystem::carriedLoad(player);
    assert(withStoredWeapon >= 5);

    // Gear currently worn/carried in hand has its own combat weight trade-off and must not be
    // double-counted as bag bulk.
    assert(player.equipWeapon(0));
    assert(CarryLoadSystem::carriedLoad(player) < withStoredWeapon);
    player.unequipWeapon();

    // Physical money uses the number of actual coins, not its exchange value.
    Inventory wallet;
    CoinBreakdown coins;
    coins.gold = 100;
    wallet.setCoinStacks(coins);
    assert(CarryLoadSystem::coinLoad(wallet) == 1);
    coins.copper = 1;
    wallet.setCoinStacks(coins);
    assert(CarryLoadSystem::coinLoad(wallet) == 2);

    // Natural racial body size and chosen morphology stack instead of replacing each other.
    Player fairyFine;
    fairyFine.setRace(CharacterRace::Fairy);
    fairyFine.setAppearanceProfile(18, "Test", "Morphologie fine");
    Player orcRobust;
    orcRobust.setRace(CharacterRace::Orc);
    orcRobust.setAppearanceProfile(18, "Test", "Morphologie robuste");
    assert(CarryLoadSystem::capacity(orcRobust) > CarryLoadSystem::capacity(fairyFine));
    assert(CarryLoadSystem::incomingAccuracyModifierPercent(orcRobust)
        > CarryLoadSystem::incomingAccuracyModifierPercent(fairyFine));

    // A pure mage naturally carries a little less. The object-centric Bibliomancer is exempt.
    Player fighter("Porteur", ClassCatalog::createClassByName("Guerrier"));
    fighter.setAppearanceProfile(18, "Test", "Morphologie standard");
    Player mage("Arcaniste", ClassCatalog::createClassByName("Mage"));
    mage.setAppearanceProfile(18, "Test", "Morphologie standard");
    Player bibliomancer("Bibli", ClassCatalog::createClassByName("Bibliomancien"));
    bibliomancer.setAppearanceProfile(18, "Test", "Morphologie standard");
    assert(CarryLoadSystem::capacity(mage) < CarryLoadSystem::capacity(fighter));
    assert(CarryLoadSystem::capacity(bibliomancer) >= CarryLoadSystem::capacity(mage));

    // Bag progression is persistent capacity progression, not a hidden replacement of body modifiers.
    const int beforeBagUpgrade = CarryLoadSystem::capacity(fighter);
    fighter.setLoadedBackpackTier(4);
    assert(fighter.getBackpackBaseCapacity() == 80);
    assert(CarryLoadSystem::capacity(fighter) > beforeBagUpgrade);

    // Thresholds are checked with a neutral standard human and only stackable generic materials.
    Player threshold;
    threshold.getInventory().clearAll();
    threshold.setAppearanceProfile(18, "Test", "Morphologie standard");
    const int capacity = CarryLoadSystem::capacity(threshold);
    assert(capacity >= 40);

    // Generic materials are 1 load per 10, so choose quantities from the actual capacity.
    threshold.getInventory().addMaterial(normalLoadMaterial((capacity + 1) * 10));
    CarryLoadSummary overloaded = CarryLoadSystem::evaluate(threshold);
    assert(overloaded.percent > 100 && overloaded.percent <= 125);
    assert(overloaded.escapeModifierPercent == -10);
    assert(overloaded.meleeAccuracyModifierPercent == -5);
    assert(overloaded.physicalProtectionPercent == 2);
    assert(overloaded.explorationTimeModifierPercent == 10);
    assert(!overloaded.movementBlocked);

    threshold.getInventory().clearAll();
    threshold.getInventory().addMaterial(normalLoadMaterial(((capacity * 3) / 2) * 10));
    overloaded = CarryLoadSystem::evaluate(threshold);
    assert(overloaded.percent > 125 && overloaded.percent < 200);
    assert(overloaded.escapeModifierPercent == -20);
    assert(overloaded.meleeAccuracyModifierPercent == -10);
    assert(overloaded.physicalProtectionPercent == 4);
    assert(overloaded.explorationTimeModifierPercent == 25);
    assert(!overloaded.movementBlocked);

    threshold.getInventory().clearAll();
    threshold.getInventory().addMaterial(normalLoadMaterial(capacity * 20));
    overloaded = CarryLoadSystem::evaluate(threshold);
    assert(overloaded.percent >= 200);
    assert(overloaded.escapeModifierPercent == -20);
    assert(overloaded.meleeAccuracyModifierPercent == -10);
    assert(overloaded.physicalProtectionPercent == 4);
    assert(overloaded.explorationTimeModifierPercent == 25);
    assert(overloaded.movementBlocked);

    return 0;
}
