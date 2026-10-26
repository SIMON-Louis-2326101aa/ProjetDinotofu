#include "world/clan/ClanHeadquartersSystem.hpp"

#include "entity/Player.hpp"
#include "item/material/MaterialCatalog.hpp"

#include <algorithm>
#include <cassert>
#include <string>

namespace
{
    std::string recruitLabel(const std::string& name, int level = 8)
    {
        return name + " | race=Humain | profil=Éclaireur | trait=fiable | niveau_recrutement=" + std::to_string(level)
            + " | rang_origine_index=1 | part_base=18 | salaire=90 | profil_seulement=1";
    }

    int count(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
            if (record.category == category && record.key == key) return record.count;
        return 0;
    }

    void addUpgradeResources(Player& player, ClanHeadquartersUpgradeKind kind)
    {
        player.getInventory().earnCopper(ClanHeadquartersSystem::upgradeCostCopper(player, kind));
        for (const ClanHeadquartersMaterialRequirement& requirement : ClanHeadquartersSystem::upgradeMaterials(player, kind))
            player.getInventory().addMaterial(MaterialCatalog::createById(requirement.materialId, requirement.quantity));
    }

    void addRecruit(Player& player, const std::string& name)
    {
        player.recordCanonicalEvent("pnj_recrutables_retenus", name, recruitLabel(name), 1);
        player.recordCanonicalEvent("recrues_engagees", name, name + " rejoint réellement le groupe", 1);
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
    player.recordCanonicalEvent("clan_hq_modules", "counter", "Comptoir du QG installé", 1);
    player.recordCanonicalEvent("clan_hq_modules", "quest_board", "Tableau de quêtes de clan niveau 1 installé", 1);
    player.recordCanonicalEvent("clan_hq_modules", "bedroom_base", "Deux chambres/lits de recrues disponibles", 1);
    player.recordCanonicalEvent("clan_hq_passive_clock", "last_check", "Horloge initialisée", 1);

    std::string reason;
    ClanHeadquartersStatus status = ClanHeadquartersSystem::statusFor(player);
    assert(status.built);
    assert(status.buildingLevel == 1);
    assert(status.bedCapacity == 2);
    assert(status.maxBedCapacity == 2);
    assert(status.moduleCapacity == 3);
    assert(!ClanHeadquartersSystem::canStartUpgrade(player, ClanHeadquartersUpgradeKind::ExtraBed, reason));
    assert(!ClanHeadquartersSystem::canStartUpgrade(player, ClanHeadquartersUpgradeKind::QuestBoard, reason));

    addUpgradeResources(player, ClanHeadquartersUpgradeKind::BuildingExpansion);
    assert(ClanHeadquartersSystem::canStartUpgrade(player, ClanHeadquartersUpgradeKind::BuildingExpansion, reason));
    assert(ClanHeadquartersSystem::startUpgrade(player, ClanHeadquartersUpgradeKind::BuildingExpansion, reason));
    ClanHeadquartersUpgradeStatus upgrade = ClanHeadquartersSystem::upgradeStatusFor(player);
    assert(upgrade.active);
    assert(upgrade.kind == ClanHeadquartersUpgradeKind::BuildingExpansion);
    assert(upgrade.targetLevel == 2);
    assert(upgrade.readyDay == player.getWorldDaysElapsed() + ClanHeadquartersSystem::BuildingLevel2Days);

    // One construction crew / one structural job at a time.
    addUpgradeResources(player, ClanHeadquartersUpgradeKind::ExtraBed);
    assert(!ClanHeadquartersSystem::canStartUpgrade(player, ClanHeadquartersUpgradeKind::ExtraBed, reason));

    player.advanceWorldDays(ClanHeadquartersSystem::BuildingLevel2Days - 1);
    assert(!ClanHeadquartersSystem::completeUpgradeIfReady(player));
    player.advanceWorldDays(1);
    std::string completion;
    assert(ClanHeadquartersSystem::completeUpgradeIfReady(player, &completion));
    assert(!completion.empty());

    status = ClanHeadquartersSystem::statusFor(player);
    assert(status.buildingLevel == 2);
    assert(status.bedCapacity == 2);
    assert(status.maxBedCapacity == ClanHeadquartersSystem::BuildingLevel2MaxBeds);
    assert(status.moduleCapacity == ClanHeadquartersSystem::BuildingLevel2ModuleCapacity);
    assert(!ClanHeadquartersSystem::upgradeStatusFor(player).active);

    // The previous attempted-bed resources were not consumed because the expansion was active.
    assert(ClanHeadquartersSystem::canStartUpgrade(player, ClanHeadquartersUpgradeKind::ExtraBed, reason));
    assert(ClanHeadquartersSystem::startUpgrade(player, ClanHeadquartersUpgradeKind::ExtraBed, reason));
    player.advanceWorldDays(ClanHeadquartersSystem::ExtraBedDays);
    assert(ClanHeadquartersSystem::completeUpgradeIfReady(player));
    status = ClanHeadquartersSystem::statusFor(player);
    assert(status.bedCapacity == 3);

    addUpgradeResources(player, ClanHeadquartersUpgradeKind::QuestBoard);
    assert(ClanHeadquartersSystem::canStartUpgrade(player, ClanHeadquartersUpgradeKind::QuestBoard, reason));
    assert(ClanHeadquartersSystem::startUpgrade(player, ClanHeadquartersUpgradeKind::QuestBoard, reason));
    player.advanceWorldDays(ClanHeadquartersSystem::QuestBoardLevel2Days);
    assert(ClanHeadquartersSystem::completeUpgradeIfReady(player));
    status = ClanHeadquartersSystem::statusFor(player);
    assert(status.questBoardLevel == 2);
    assert(!ClanHeadquartersSystem::canStartUpgrade(player, ClanHeadquartersUpgradeKind::QuestBoard, reason));

    addUpgradeResources(player, ClanHeadquartersUpgradeKind::MissionChest);
    assert(ClanHeadquartersSystem::canStartUpgrade(player, ClanHeadquartersUpgradeKind::MissionChest, reason));
    assert(ClanHeadquartersSystem::startUpgrade(player, ClanHeadquartersUpgradeKind::MissionChest, reason));
    player.advanceWorldDays(ClanHeadquartersSystem::MissionChestLevel1Days);
    assert(ClanHeadquartersSystem::completeUpgradeIfReady(player));
    status = ClanHeadquartersSystem::statusFor(player);
    assert(status.missionChestLevel == 1);
    assert(status.missionChestCapacityCopper == ClanHeadquartersSystem::MissionChestLevel1CapacityCopper);
    assert(status.modulesUsed == 4);

    // Phase 3 cannot be bought with money alone: one optional module is not enough.
    addUpgradeResources(player, ClanHeadquartersUpgradeKind::BuildingExpansion);
    assert(!ClanHeadquartersSystem::canStartUpgrade(player, ClanHeadquartersUpgradeKind::BuildingExpansion, reason));

    // A second real service module completes the structural prerequisite.
    addUpgradeResources(player, ClanHeadquartersUpgradeKind::CommonRoomBar);
    assert(ClanHeadquartersSystem::canStartUpgrade(player, ClanHeadquartersUpgradeKind::CommonRoomBar, reason));
    assert(ClanHeadquartersSystem::startUpgrade(player, ClanHeadquartersUpgradeKind::CommonRoomBar, reason));
    player.advanceWorldDays(ClanHeadquartersSystem::CommonRoomBarLevel1Days);
    assert(ClanHeadquartersSystem::completeUpgradeIfReady(player));
    status = ClanHeadquartersSystem::statusFor(player);
    assert(status.commonRoomBarLevel == 1);
    assert(status.modulesUsed == 5);

    // Resources for the previously blocked expansion remained untouched, and the real level-3 quote is more expensive.
    assert(ClanHeadquartersSystem::upgradeCostCopper(player, ClanHeadquartersUpgradeKind::BuildingExpansion) == ClanHeadquartersSystem::BuildingLevel3CostCopper);
    for (const ClanHeadquartersMaterialRequirement& requirement : ClanHeadquartersSystem::upgradeMaterials(player, ClanHeadquartersUpgradeKind::BuildingExpansion))
    {
        const int missing = std::max(0, requirement.quantity - player.getInventory().countMaterialById(requirement.materialId));
        if (missing > 0) player.getInventory().addMaterial(MaterialCatalog::createById(requirement.materialId, missing));
    }
    if (player.getInventory().getTotalCopper() < ClanHeadquartersSystem::BuildingLevel3CostCopper)
        player.getInventory().earnCopper(ClanHeadquartersSystem::BuildingLevel3CostCopper - player.getInventory().getTotalCopper());
    assert(ClanHeadquartersSystem::canStartUpgrade(player, ClanHeadquartersUpgradeKind::BuildingExpansion, reason));
    assert(ClanHeadquartersSystem::startUpgrade(player, ClanHeadquartersUpgradeKind::BuildingExpansion, reason));
    ClanHeadquartersUpgradeStatus phaseThree = ClanHeadquartersSystem::upgradeStatusFor(player);
    assert(phaseThree.targetLevel == 3);
    assert(phaseThree.readyDay == player.getWorldDaysElapsed() + ClanHeadquartersSystem::BuildingLevel3Days);
    player.advanceWorldDays(ClanHeadquartersSystem::BuildingLevel3Days);
    assert(ClanHeadquartersSystem::completeUpgradeIfReady(player));
    status = ClanHeadquartersSystem::statusFor(player);
    assert(status.buildingLevel == 3);
    assert(status.maxBedCapacity == ClanHeadquartersSystem::BuildingLevel3MaxBeds);
    assert(status.moduleCapacity == ClanHeadquartersSystem::BuildingLevel3ModuleCapacity);
    assert(!ClanHeadquartersSystem::canStartUpgrade(player, ClanHeadquartersUpgradeKind::BuildingExpansion, reason)); // Levels 4/5 planned, prerequisites not opened yet.

    addRecruit(player, "Aline");
    addRecruit(player, "Boros");
    addRecruit(player, "Celia");
    assert(ClanHeadquartersSystem::assignRecruit(player, "Aline", reason));
    assert(ClanHeadquartersSystem::assignRecruit(player, "Boros", reason));
    assert(ClanHeadquartersSystem::assignRecruit(player, "Celia", reason));
    assert(ClanHeadquartersSystem::residentRecruitNames(player).size() == 3);

    // Level-2 board: autonomous XP is now 15-25% of an equivalent active quest.
    // Three rank-E residents form a group, so they may legally take rank-D contracts (+1 maximum).
    player.recordCanonicalEvent("clan_hq_passive_clock", "last_check", "Horloge remise à jour pour le test", 1);
    const int alineXpBefore = count(player, "xp_recrues", "Aline");
    player.advanceWorldDays(3);
    ClanHeadquartersPassiveReport report = ClanHeadquartersSystem::processPassiveActivity(player);
    assert(report.missionCycles == 1);
    assert(report.missionsCompleted == 3);
    assert(report.totalRecruitXp > 21);
    assert(count(player, "xp_recrues", "Aline") > alineXpBefore + 7);
    assert(report.generatedMissionCopper > 0);
    assert(report.storedMissionCopper > 0);

    status = ClanHeadquartersSystem::statusFor(player);
    assert(status.missionChestStoredCopper == report.storedMissionCopper);
    const long long copperBeforeCollection = player.getInventory().getTotalCopper();
    long long collected = 0;
    assert(ClanHeadquartersSystem::collectMissionChest(player, collected, reason));
    assert(collected == report.storedMissionCopper);
    assert(player.getInventory().getTotalCopper() == copperBeforeCollection + collected);
    assert(ClanHeadquartersSystem::statusFor(player).missionChestStoredCopper == 0);

    return 0;
}
