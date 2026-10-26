#include "world/clan/ClanHeadquartersSystem.hpp"

#include "entity/Player.hpp"
#include "item/material/MaterialCatalog.hpp"

#include <cassert>
#include <string>

namespace
{
    int count(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
            if (record.category == category && record.key == key) return record.count;
        return 0;
    }

    void addUpgradeResources(Player& player, ClanHeadquartersUpgradeKind kind)
    {
        player.getInventory().earnCopper(ClanHeadquartersSystem::upgradeCostCopper(kind));
        for (const ClanHeadquartersMaterialRequirement& requirement : ClanHeadquartersSystem::upgradeMaterials(kind))
            player.getInventory().addMaterial(MaterialCatalog::createById(requirement.materialId, requirement.quantity));
    }
}

int main()
{
    Player player;
    player.recordCanonicalEvent(
        "clan_hq_state",
        "built",
        "city=valebrume | built_day=0 | building_level=1 | beds=2 | quest_board=1 | counter=1",
        1
    );
    player.recordCanonicalEvent("clan_hq_building_expansions", "level_2", "Agrandissement général terminé : QG niveau 2", 1);
    player.recordCanonicalEvent("clan_hq_modules", "counter", "Comptoir du QG installé", 1);
    player.recordCanonicalEvent("clan_hq_modules", "quest_board", "Tableau de quêtes niveau 1", 1);
    player.recordCanonicalEvent("clan_hq_modules", "bedroom_base", "Deux chambres", 1);

    std::string reason;
    ClanHeadquartersStatus status = ClanHeadquartersSystem::statusFor(player);
    assert(status.buildingLevel == 2);
    assert(status.commonRoomBarLevel == 0);

    addUpgradeResources(player, ClanHeadquartersUpgradeKind::CommonRoomBar);
    assert(ClanHeadquartersSystem::canStartUpgrade(player, ClanHeadquartersUpgradeKind::CommonRoomBar, reason));
    assert(ClanHeadquartersSystem::startUpgrade(player, ClanHeadquartersUpgradeKind::CommonRoomBar, reason));
    player.advanceWorldDays(ClanHeadquartersSystem::CommonRoomBarLevel1Days);
    assert(ClanHeadquartersSystem::completeUpgradeIfReady(player));

    status = ClanHeadquartersSystem::statusFor(player);
    assert(status.commonRoomBarLevel == 1);
    assert(status.modulesUsed == 4);

    player.takeDamage(30);
    const int hpBefore = player.getHp();
    player.getInventory().earnCopper(ClanHeadquartersSystem::CommonRoomBarVisitCostCopper * 2);
    const long long copperBefore = player.getInventory().getTotalCopper();

    int healed = 0;
    long long paid = 0;
    assert(ClanHeadquartersSystem::useCommonRoomBar(player, healed, paid, reason));
    assert(healed > 0);
    assert(player.getHp() == hpBefore + healed);
    assert(paid == ClanHeadquartersSystem::CommonRoomBarVisitCostCopper);
    assert(player.getInventory().getTotalCopper() == copperBefore - paid);
    assert(count(player, "clan_hq_bar_visits", "day_" + std::to_string(player.getWorldDaysElapsed())) == 1);
    assert(count(player, "clan_hq_social_moments", "bar") == 1);

    // One paid recovery / social service per world day: no infinite healing loop.
    assert(!ClanHeadquartersSystem::useCommonRoomBar(player, healed, paid, reason));
    player.advanceWorldDays(1);
    assert(ClanHeadquartersSystem::useCommonRoomBar(player, healed, paid, reason));

    return 0;
}
