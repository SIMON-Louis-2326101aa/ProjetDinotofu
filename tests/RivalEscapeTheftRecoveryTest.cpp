#include <cassert>

#include "combat/profile/MonsterBehaviorProfile.hpp"
#include "entity/Monster.hpp"
#include "entity/Player.hpp"
#include "item/material/Material.hpp"

int main()
{
    Monster thief(
        "Brigand voleur",
        "Humain brigand opportuniste",
        Race::Humain,
        7,
        120,
        6,
        12,
        18,
        0,
        0
    );
    const MonsterBehaviorProfile thiefProfile = MonsterBehaviorProfileCatalog::build(thief);
    assert(thiefProfile.canFlee);
    assert(thiefProfile.canStealOnEscape);

    Monster mindless(
        "Golem de pierre",
        "Construction massive",
        Race::Construction,
        7,
        200,
        7,
        14,
        20,
        0,
        0
    );
    assert(!MonsterBehaviorProfileCatalog::build(mindless).canStealOnEscape);

    Player player;
    const std::string rivalId = player.createRivalFromEnemy(
        thief.getName(), thief.getType(), thief.getLevel(), thief.getMaxHp(), thief.getMaxDamage(),
        "Vol commis pendant une fuite réussie", "voleur opportuniste", 100);

    Material stolen(
        "runic_iron_shard",
        "Éclat de fer runique",
        "Un matériau ordinaire réellement emporté par un rival.",
        "Matériau",
        3,
        2,
        "high"
    );
    player.recordRivalStolenCopper(rivalId, 17);
    player.recordRivalStolenMaterial(rivalId, stolen, 2);
    assert(player.rivalHasStolenProperty(rivalId));

    const long long moneyBefore = player.getInventory().getTotalCopper();
    const int materialBefore = player.getInventory().countMaterialByIdAndQuality("runic_iron_shard", "high");

    player.markRivalDefeated(rivalId, "route_test");
    assert(!player.rivalHasStolenProperty(rivalId));
    assert(player.getInventory().getTotalCopper() == moneyBefore + 17);
    assert(player.getInventory().countMaterialByIdAndQuality("runic_iron_shard", "high") == materialBefore + 2);
    assert(player.hasTitle("Rends-moi ça."));

    // Recovery is idempotent: defeating/reading the same rival twice cannot duplicate the stash.
    assert(!player.recoverRivalStolenProperty(rivalId));
    assert(player.getInventory().getTotalCopper() == moneyBefore + 17);
    assert(player.getInventory().countMaterialByIdAndQuality("runic_iron_shard", "high") == materialBefore + 2);

    return 0;
}
