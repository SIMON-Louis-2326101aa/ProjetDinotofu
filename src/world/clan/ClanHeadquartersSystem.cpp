#include "world/clan/ClanHeadquartersSystem.hpp"

#include "combat/ally/RecruitedAllyCombatRoster.hpp"
#include "entity/Player.hpp"
#include "item/Inventory.hpp"
#include "quest/GuildQuestAccessPolicy.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace
{
    int recordCount(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key) return record.count;
        }
        return 0;
    }

    const PlayerJournalRecord* findRecord(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key) return &record;
        }
        return nullptr;
    }

    int intField(const std::string& label, const std::string& fieldName, int fallback)
    {
        const std::string marker = fieldName + "=";
        const std::size_t position = label.find(marker);
        if (position == std::string::npos) return fallback;
        std::size_t start = position + marker.size();
        std::size_t end = start;
        if (end < label.size() && label[end] == '-') ++end;
        while (end < label.size() && std::isdigit(static_cast<unsigned char>(label[end]))) ++end;
        if (end <= start) return fallback;
        try { return std::stoi(label.substr(start, end - start)); }
        catch (...) { return fallback; }
    }

    std::string stringField(const std::string& label, const std::string& fieldName, const std::string& fallback)
    {
        const std::string marker = fieldName + "=";
        const std::size_t position = label.find(marker);
        if (position == std::string::npos) return fallback;
        const std::size_t start = position + marker.size();
        std::size_t end = label.find(" | ", start);
        if (end == std::string::npos) end = label.size();
        if (end <= start) return fallback;
        return label.substr(start, end - start);
    }

    bool boolField(const std::string& label, const std::string& fieldName, bool fallback)
    {
        return intField(label, fieldName, fallback ? 1 : 0) != 0;
    }

    ClanHeadquartersUpgradeKind upgradeKindFromId(const std::string& id)
    {
        if (id == "building_expansion") return ClanHeadquartersUpgradeKind::BuildingExpansion;
        if (id == "extra_bed") return ClanHeadquartersUpgradeKind::ExtraBed;
        if (id == "quest_board") return ClanHeadquartersUpgradeKind::QuestBoard;
        if (id == "mission_chest") return ClanHeadquartersUpgradeKind::MissionChest;
        if (id == "common_room_bar") return ClanHeadquartersUpgradeKind::CommonRoomBar;
        if (id == "comfort_decor") return ClanHeadquartersUpgradeKind::ComfortDecor;
        if (id == "forge_workshop") return ClanHeadquartersUpgradeKind::ForgeWorkshop;
        return ClanHeadquartersUpgradeKind::None;
    }

    std::string upgradeKindId(ClanHeadquartersUpgradeKind kind)
    {
        switch (kind)
        {
            case ClanHeadquartersUpgradeKind::BuildingExpansion: return "building_expansion";
            case ClanHeadquartersUpgradeKind::ExtraBed: return "extra_bed";
            case ClanHeadquartersUpgradeKind::QuestBoard: return "quest_board";
            case ClanHeadquartersUpgradeKind::MissionChest: return "mission_chest";
            case ClanHeadquartersUpgradeKind::CommonRoomBar: return "common_room_bar";
            case ClanHeadquartersUpgradeKind::ComfortDecor: return "comfort_decor";
            case ClanHeadquartersUpgradeKind::ForgeWorkshop: return "forge_workshop";
            case ClanHeadquartersUpgradeKind::None: break;
        }
        return "none";
    }

    bool hasNamedClan(const Player& player)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == "clan_nom" && record.key == "nom_actuel" && !record.label.empty()) return true;
        }
        return false;
    }

    bool groupTestPassed(const Player& player)
    {
        return player.getCanonicalJournalCategoryTotal("tests_groupe_guilde_reussis") > 0;
    }

    bool recruitDismissed(const Player& player, const std::string& name)
    {
        return recordCount(player, "recrues_renvoyees", name) > 0;
    }

    bool recruitActuallyRecruited(const Player& player, const PlayerJournalRecord& record)
    {
        if (!boolField(record.label, "profil_seulement", false)) return true;
        return recordCount(player, "recrues_engagees", record.key) > 0;
    }

    bool recruitEquipped(const Player& player, const std::string& name)
    {
        return (recordCount(player, "recrues_equipees_toggle", name) % 2) == 1;
    }

    bool recruitUnavailable(const Player& player, const std::string& name)
    {
        const bool infirmary = recordCount(player, "recrues_infirmerie_sejours", name)
            > recordCount(player, "recrues_infirmerie_recuperees", name);
        const bool awaitingTransfer = recordCount(player, "recrues_a_evacuer", name)
            > recordCount(player, "recrues_evacuees_infirmerie", name);
        return infirmary || awaitingTransfer;
    }

    unsigned int stableHash(const std::string& seed)
    {
        unsigned int hash = 2166136261u;
        for (unsigned char c : seed)
        {
            hash ^= c;
            hash *= 16777619u;
        }
        return hash;
    }

    struct RecruitMissionProfile
    {
        int rankIndex = 0;
        int estimatedLevel = 1;
    };

    RecruitMissionProfile missionProfileFor(const Player& player, const std::string& name)
    {
        const auto active = RecruitedAllyCombatRoster::collectActive(player);
        for (const RecruitedAllyCombatSupport& ally : active)
        {
            if (ally.name == name) return {ally.currentRankIndex, ally.estimatedLevel};
        }
        const auto reserve = RecruitedAllyCombatRoster::collectReserve(player);
        for (const RecruitedAllyCombatSupport& ally : reserve)
        {
            if (ally.name == name) return {ally.currentRankIndex, ally.estimatedLevel};
        }
        return {};
    }

    std::string recruitRankName(int index)
    {
        static const std::vector<std::string> ranks = {"F", "E", "D", "C", "B", "A", "S"};
        index = std::clamp(index, 0, static_cast<int>(ranks.size()) - 1);
        return ranks[static_cast<std::size_t>(index)];
    }

    int typicalActiveQuestExperience(int rankIndex, int level)
    {
        static const int rankPower[] = {1, 2, 3, 4, 5, 7, 10};
        rankIndex = std::clamp(rankIndex, 0, 6);
        level = std::max(1, level);
        const int typicalTarget = 2 + std::min(4, rankIndex / 2);
        return 10 + level * 5 + typicalTarget * 6 + rankPower[rankIndex] * 10;
    }

    int passiveExperiencePercent(const std::string& recruitName, int currentDay, int cycleIndex)
    {
        return 15 + static_cast<int>(stableHash(recruitName + ":clan_xp:" + std::to_string(currentDay) + ":" + std::to_string(cycleIndex)) % 11u);
    }

    long long passiveMissionCopper(int questRankIndex, int estimatedLevel)
    {
        // Deliberately small clan share: the adventurer keeps most of their own pay/equipment costs.
        return std::max<long long>(4, 5 + questRankIndex * 5 + estimatedLevel / 4);
    }

    bool hasMaterialsFor(const Player& player, const std::vector<ClanHeadquartersMaterialRequirement>& requirements, std::string& reason)
    {
        for (const ClanHeadquartersMaterialRequirement& requirement : requirements)
        {
            if (player.getInventory().countMaterialById(requirement.materialId) < requirement.quantity)
            {
                reason = "Matériaux insuffisants : " + requirement.label + ".";
                return false;
            }
        }
        return true;
    }
}

std::vector<ClanHeadquartersMaterialRequirement> ClanHeadquartersSystem::baseConstructionMaterials()
{
    return {
        {"sun_dried_clay", "Argile rouge séchée", 36},
        {"rusted_metal_fragment", "Fragments de métal utilisables", 28},
        {"worn_leather_piece", "Pièces de cuir / attaches", 16},
        {"salted_rope_knot", "Cordages solides", 10}
    };
}

std::vector<ClanHeadquartersMaterialRequirement> ClanHeadquartersSystem::upgradeMaterials(ClanHeadquartersUpgradeKind kind)
{
    switch (kind)
    {
        case ClanHeadquartersUpgradeKind::BuildingExpansion:
            return {
                {"sun_dried_clay", "Argile rouge séchée", 72},
                {"rusted_metal_fragment", "Fragments de métal utilisables", 58},
                {"worn_leather_piece", "Pièces de cuir / attaches", 30},
                {"salted_rope_knot", "Cordages solides", 20}
            };
        case ClanHeadquartersUpgradeKind::ExtraBed:
            return {
                {"sun_dried_clay", "Argile rouge séchée", 8},
                {"rusted_metal_fragment", "Fragments de métal utilisables", 8},
                {"worn_leather_piece", "Pièces de cuir / literie", 12},
                {"salted_rope_knot", "Cordages solides", 6}
            };
        case ClanHeadquartersUpgradeKind::QuestBoard:
            return {
                {"sun_dried_clay", "Argile rouge séchée", 10},
                {"rusted_metal_fragment", "Ferrures / métal utilisable", 14},
                {"worn_leather_piece", "Cuir pour pochettes et registres", 8},
                {"salted_rope_knot", "Cordages solides", 6}
            };
        case ClanHeadquartersUpgradeKind::MissionChest:
            return {
                {"sun_dried_clay", "Argile rouge séchée", 12},
                {"rusted_metal_fragment", "Métal pour coffre et serrurerie", 24},
                {"worn_leather_piece", "Cuir pour registres / sacoches", 8},
                {"salted_rope_knot", "Cordages de manutention", 4}
            };
        case ClanHeadquartersUpgradeKind::CommonRoomBar:
            return {
                {"sun_dried_clay", "Argile / maçonnerie de la salle commune", 14},
                {"rusted_metal_fragment", "Ferrures, comptoir et équipements", 18},
                {"worn_leather_piece", "Assises et garnitures", 16},
                {"salted_rope_knot", "Cordages / suspensions", 6}
            };
        case ClanHeadquartersUpgradeKind::ComfortDecor:
            return {
                {"sun_dried_clay", "Petites finitions / cloisons", 8},
                {"rusted_metal_fragment", "Ferrures et supports de mobilier", 6},
                {"worn_leather_piece", "Literie, coussins et garnitures", 18},
                {"salted_rope_knot", "Suspensions et attaches décoratives", 8}
            };
        case ClanHeadquartersUpgradeKind::ForgeWorkshop:
            return {
                {"sun_dried_clay", "Briques réfractaires / maçonnerie de forge", 24},
                {"rusted_metal_fragment", "Métal pour enclume, outils et foyer", 46},
                {"worn_leather_piece", "Soufflets, tabliers et protections", 14},
                {"salted_rope_knot", "Cordages de levage et de manutention", 8}
            };
        case ClanHeadquartersUpgradeKind::None:
            break;
    }
    return {};
}

std::vector<ClanHeadquartersMaterialRequirement> ClanHeadquartersSystem::upgradeMaterials(const Player& player, ClanHeadquartersUpgradeKind kind)
{
    if (kind == ClanHeadquartersUpgradeKind::BuildingExpansion && statusFor(player).buildingLevel == 2)
    {
        return {
            {"sun_dried_clay", "Argile rouge séchée", 120},
            {"rusted_metal_fragment", "Métal pour charpente, renforts et nouvelles ailes", 100},
            {"worn_leather_piece", "Cuir / garnitures / isolations", 50},
            {"salted_rope_knot", "Cordages de chantier lourds", 36}
        };
    }
    return upgradeMaterials(kind);
}

long long ClanHeadquartersSystem::upgradeCostCopper(ClanHeadquartersUpgradeKind kind)
{
    switch (kind)
    {
        case ClanHeadquartersUpgradeKind::BuildingExpansion: return BuildingLevel2CostCopper;
        case ClanHeadquartersUpgradeKind::ExtraBed: return ExtraBedCostCopper;
        case ClanHeadquartersUpgradeKind::QuestBoard: return QuestBoardLevel2CostCopper;
        case ClanHeadquartersUpgradeKind::MissionChest: return MissionChestLevel1CostCopper;
        case ClanHeadquartersUpgradeKind::CommonRoomBar: return CommonRoomBarLevel1CostCopper;
        case ClanHeadquartersUpgradeKind::ComfortDecor: return ComfortDecorLevel1CostCopper;
        case ClanHeadquartersUpgradeKind::ForgeWorkshop: return ForgeWorkshopLevel1CostCopper;
        case ClanHeadquartersUpgradeKind::None: break;
    }
    return 0;
}

long long ClanHeadquartersSystem::upgradeCostCopper(const Player& player, ClanHeadquartersUpgradeKind kind)
{
    if (kind == ClanHeadquartersUpgradeKind::BuildingExpansion && statusFor(player).buildingLevel == 2)
        return BuildingLevel3CostCopper;
    return upgradeCostCopper(kind);
}

int ClanHeadquartersSystem::upgradeDurationDays(ClanHeadquartersUpgradeKind kind)
{
    switch (kind)
    {
        case ClanHeadquartersUpgradeKind::BuildingExpansion: return BuildingLevel2Days;
        case ClanHeadquartersUpgradeKind::ExtraBed: return ExtraBedDays;
        case ClanHeadquartersUpgradeKind::QuestBoard: return QuestBoardLevel2Days;
        case ClanHeadquartersUpgradeKind::MissionChest: return MissionChestLevel1Days;
        case ClanHeadquartersUpgradeKind::CommonRoomBar: return CommonRoomBarLevel1Days;
        case ClanHeadquartersUpgradeKind::ComfortDecor: return ComfortDecorLevel1Days;
        case ClanHeadquartersUpgradeKind::ForgeWorkshop: return ForgeWorkshopLevel1Days;
        case ClanHeadquartersUpgradeKind::None: break;
    }
    return 0;
}

int ClanHeadquartersSystem::upgradeDurationDays(const Player& player, ClanHeadquartersUpgradeKind kind)
{
    if (kind == ClanHeadquartersUpgradeKind::BuildingExpansion && statusFor(player).buildingLevel == 2)
        return BuildingLevel3Days;
    return upgradeDurationDays(kind);
}

std::string ClanHeadquartersSystem::upgradeDisplayName(ClanHeadquartersUpgradeKind kind)
{
    switch (kind)
    {
        case ClanHeadquartersUpgradeKind::BuildingExpansion: return "Agrandissement général du QG";
        case ClanHeadquartersUpgradeKind::ExtraBed: return "Nouvelle chambre / lit de recrue";
        case ClanHeadquartersUpgradeKind::QuestBoard: return "Tableau de quêtes niveau 2";
        case ClanHeadquartersUpgradeKind::MissionChest: return "Caisse de missions du clan";
        case ClanHeadquartersUpgradeKind::CommonRoomBar: return "Bar / salle commune du clan";
        case ClanHeadquartersUpgradeKind::ComfortDecor: return "Confort & décoration du QG niveau 1";
        case ClanHeadquartersUpgradeKind::ForgeWorkshop: return "Atelier de forge du clan niveau 1";
        case ClanHeadquartersUpgradeKind::None: break;
    }
    return "Aucune amélioration";
}

ClanHeadquartersStatus ClanHeadquartersSystem::statusFor(const Player& player)
{
    ClanHeadquartersStatus status;
    const PlayerJournalRecord* start = findRecord(player, "clan_hq_construction", "base");
    const PlayerJournalRecord* built = findRecord(player, "clan_hq_state", "built");

    if (start != nullptr)
    {
        status.constructionStarted = true;
        status.cityId = stringField(start->label, "city", start->locationId);
        status.constructionStartedDay = intField(start->label, "started_day", start->lastDay);
        status.constructionReadyDay = intField(start->label, "ready_day", status.constructionStartedDay + BaseConstructionDays);
    }
    if (built != nullptr && built->count > 0)
    {
        status.built = true;
        status.cityId = stringField(built->label, "city", status.cityId.empty() ? built->locationId : status.cityId);
        status.buildingLevel = std::clamp(1 + player.getCanonicalJournalCategoryTotal("clan_hq_building_expansions"), 1, MaxBuildingLevel);
        status.bedCapacity = BaseBedCapacity + player.getCanonicalJournalCategoryTotal("clan_hq_extra_beds");
        if (status.buildingLevel >= 3)
        {
            status.maxBedCapacity = BuildingLevel3MaxBeds;
            status.moduleCapacity = BuildingLevel3ModuleCapacity;
        }
        else if (status.buildingLevel >= 2)
        {
            status.maxBedCapacity = BuildingLevel2MaxBeds;
            status.moduleCapacity = BuildingLevel2ModuleCapacity;
        }
        else
        {
            status.maxBedCapacity = BaseBedCapacity;
            status.moduleCapacity = BaseModuleCapacity;
        }
        status.questBoardLevel = 1 + player.getCanonicalJournalCategoryTotal("clan_hq_quest_board_upgrades");
        status.missionChestLevel = player.getCanonicalJournalCategoryTotal("clan_hq_mission_chest_upgrades");
        status.commonRoomBarLevel = player.getCanonicalJournalCategoryTotal("clan_hq_common_room_bar_upgrades");
        status.comfortDecorLevel = player.getCanonicalJournalCategoryTotal("clan_hq_comfort_decor_upgrades");
        status.forgeWorkshopLevel = player.getCanonicalJournalCategoryTotal("clan_hq_forge_workshop_upgrades");
        status.missionChestCapacityCopper = status.missionChestLevel > 0 ? MissionChestLevel1CapacityCopper : 0;
        const long long deposited = std::max(0, recordCount(player, "clan_hq_mission_chest_deposits", "funds"));
        const long long withdrawn = std::max(0, recordCount(player, "clan_hq_mission_chest_withdrawals", "funds"));
        status.missionChestStoredCopper = std::clamp<long long>(deposited - withdrawn, 0, status.missionChestCapacityCopper);
        status.modulesUsed = BaseModuleCapacity
            + (status.missionChestLevel > 0 ? 1 : 0)
            + (status.commonRoomBarLevel > 0 ? 1 : 0)
            + (status.forgeWorkshopLevel > 0 ? 1 : 0);
        status.counterAvailable = true;
    }
    return status;
}

ClanHeadquartersUpgradeStatus ClanHeadquartersSystem::upgradeStatusFor(const Player& player)
{
    ClanHeadquartersUpgradeStatus status;
    const PlayerJournalRecord* started = findRecord(player, "clan_hq_upgrade_jobs_started", "active");
    if (started == nullptr) return status;

    const int completed = recordCount(player, "clan_hq_upgrade_jobs_completed", "active");
    if (started->count <= completed) return status;

    status.active = true;
    status.kind = upgradeKindFromId(stringField(started->label, "kind", "none"));
    status.targetLevel = intField(started->label, "target", 0);
    status.startedDay = intField(started->label, "started_day", started->lastDay);
    status.readyDay = intField(started->label, "ready_day", status.startedDay + upgradeDurationDays(status.kind));
    return status;
}

bool ClanHeadquartersSystem::canStartConstruction(const Player& player, std::string& reason)
{
    const ClanHeadquartersStatus status = statusFor(player);
    if (status.built)
    {
        reason = "Ce personnage possède déjà son unique QG de clan.";
        return false;
    }
    if (status.constructionStarted)
    {
        reason = "La construction de l'unique QG est déjà lancée.";
        return false;
    }
    if (!groupTestPassed(player) || !hasNamedClan(player))
    {
        reason = "Le QG demande d'abord le test de groupe de la guilde et un nom de clan enregistré.";
        return false;
    }
    if (player.getInventory().getTotalCopper() < BaseConstructionCostCopper)
    {
        reason = "Fonds insuffisants pour lancer le chantier.";
        return false;
    }
    if (!hasMaterialsFor(player, baseConstructionMaterials(), reason)) return false;
    reason = "Le chantier peut être lancé.";
    return true;
}

bool ClanHeadquartersSystem::startConstruction(Player& player, std::string& reason)
{
    if (!canStartConstruction(player, reason)) return false;

    if (!player.getInventory().spendCopper(BaseConstructionCostCopper))
    {
        reason = "Le paiement du chantier a échoué.";
        return false;
    }

    for (const ClanHeadquartersMaterialRequirement& requirement : baseConstructionMaterials())
    {
        if (!player.getInventory().removeMaterialQuantityById(requirement.materialId, requirement.quantity))
        {
            player.getInventory().earnCopper(BaseConstructionCostCopper);
            reason = "Le chantier n'a pas pu réserver tous les matériaux. Le paiement a été rendu.";
            return false;
        }
    }

    const int startedDay = player.getWorldDaysElapsed();
    const int readyDay = startedDay + BaseConstructionDays;
    std::ostringstream label;
    label << "city=" << player.getCurrentCityId()
          << " | started_day=" << startedDay
          << " | ready_day=" << readyDay
          << " | base_beds=" << BaseBedCapacity
          << " | base_board=1 | counter=1";
    player.recordCanonicalEvent("clan_hq_construction", "base", label.str(), 1);
    player.recordCanonicalEvent("clan_hq_investment", "base", "Construction initiale du QG", 1);
    reason = "Construction lancée. Le QG de base demande plusieurs jours de chantier.";
    return true;
}

bool ClanHeadquartersSystem::completeConstructionIfReady(Player& player)
{
    ClanHeadquartersStatus status = statusFor(player);
    if (status.built || !status.constructionStarted || status.constructionReadyDay < 0) return false;
    if (player.getWorldDaysElapsed() < status.constructionReadyDay) return false;

    std::ostringstream label;
    label << "city=" << status.cityId
          << " | built_day=" << player.getWorldDaysElapsed()
          << " | building_level=1 | beds=2 | quest_board=1 | counter=1";
    player.recordCanonicalEvent("clan_hq_state", "built", label.str(), 1);
    player.recordCanonicalEvent("clan_hq_modules", "counter", "Comptoir du QG installé", 1);
    player.recordCanonicalEvent("clan_hq_modules", "quest_board", "Tableau de quêtes de clan niveau 1 installé", 1);
    player.recordCanonicalEvent("clan_hq_modules", "bedroom_base", "Deux chambres/lits de recrues disponibles", 1);
    player.recordCanonicalEvent("clan_hq_passive_clock", "last_check", "Horloge d'activité du QG initialisée", 1);
    return true;
}

bool ClanHeadquartersSystem::canStartUpgrade(const Player& player, ClanHeadquartersUpgradeKind kind, std::string& reason)
{
    const ClanHeadquartersStatus status = statusFor(player);
    if (!status.built)
    {
        reason = "Le QG doit d'abord être construit.";
        return false;
    }
    if (player.getCurrentCityId() != status.cityId)
    {
        reason = "Les travaux doivent être commandés depuis la ville où se trouve le QG.";
        return false;
    }
    if (upgradeStatusFor(player).active)
    {
        reason = "Un chantier d'amélioration est déjà en cours au QG.";
        return false;
    }

    switch (kind)
    {
        case ClanHeadquartersUpgradeKind::BuildingExpansion:
            if (status.buildingLevel >= MaxBuildingLevel)
            {
                reason = "Le QG a déjà atteint le niveau structurel maximum prévu (5).";
                return false;
            }
            if (status.buildingLevel == 2)
            {
                const int optionalModules = std::max(0, status.modulesUsed - BaseModuleCapacity);
                if (status.questBoardLevel < 2)
                {
                    reason = "Le passage au QG niveau 3 demande au minimum un tableau de quêtes niveau 2.";
                    return false;
                }
                if (optionalModules < 2)
                {
                    reason = "Le passage au QG niveau 3 demande au moins deux vrais modules de service installés (caisse, bar, forge...).";
                    return false;
                }
                if (status.bedCapacity < 3)
                {
                    reason = "Le passage au QG niveau 3 demande au moins une chambre supplémentaire : le clan doit déjà avoir réellement commencé à grandir.";
                    return false;
                }
            }
            else if (status.buildingLevel >= 3)
            {
                reason = "Les phases structurelles 4 et 5 sont bien prévues comme plafond final, mais leurs prérequis de modules supérieurs ne sont pas encore ouverts.";
                return false;
            }
            break;
        case ClanHeadquartersUpgradeKind::ExtraBed:
            if (status.buildingLevel < 2)
            {
                reason = "Le QG de base est plein : agrandis d'abord le bâtiment au niveau 2.";
                return false;
            }
            if (status.bedCapacity >= status.maxBedCapacity)
            {
                reason = "La capacité de chambres actuelle est déjà au maximum pour cette taille de bâtiment.";
                return false;
            }
            break;
        case ClanHeadquartersUpgradeKind::QuestBoard:
            if (status.buildingLevel < 2)
            {
                reason = "Le tableau niveau 2 demande d'abord un QG agrandi.";
                return false;
            }
            if (status.questBoardLevel >= 2)
            {
                reason = "Le prochain niveau de tableau n'est pas encore disponible dans cette version.";
                return false;
            }
            break;
        case ClanHeadquartersUpgradeKind::MissionChest:
            if (status.buildingLevel < 2 || status.questBoardLevel < 2)
            {
                reason = "La caisse de missions demande un QG niveau 2 et un tableau de quêtes niveau 2.";
                return false;
            }
            if (status.missionChestLevel >= 1)
            {
                reason = "La caisse de missions niveau 1 est déjà installée.";
                return false;
            }
            if (status.modulesUsed >= status.moduleCapacity)
            {
                reason = "Aucun emplacement de module n'est disponible dans le bâtiment actuel.";
                return false;
            }
            break;
        case ClanHeadquartersUpgradeKind::CommonRoomBar:
            if (status.buildingLevel < 2)
            {
                reason = "Le bar / salle commune demande un QG agrandi au niveau 2.";
                return false;
            }
            if (status.commonRoomBarLevel >= 1)
            {
                reason = "Le bar / salle commune niveau 1 est déjà installé.";
                return false;
            }
            if (status.modulesUsed >= status.moduleCapacity)
            {
                reason = "Aucun emplacement de module n'est disponible dans le bâtiment actuel.";
                return false;
            }
            break;
        case ClanHeadquartersUpgradeKind::ComfortDecor:
            if (status.buildingLevel < 2)
            {
                reason = "Le confort / la décoration niveau 1 demande d'abord un QG agrandi au niveau 2.";
                return false;
            }
            if (status.comfortDecorLevel >= 1)
            {
                reason = "Le confort / la décoration niveau 1 est déjà installé.";
                return false;
            }
            // Furnishings improve existing rooms and therefore do not consume a workshop/module slot.
            break;
        case ClanHeadquartersUpgradeKind::ForgeWorkshop:
            if (status.buildingLevel < 2)
            {
                reason = "L'atelier de forge demande un QG agrandi au niveau 2.";
                return false;
            }
            if (status.forgeWorkshopLevel >= 1)
            {
                reason = "L'atelier de forge niveau 1 est déjà construit.";
                return false;
            }
            if (status.modulesUsed >= status.moduleCapacity)
            {
                reason = "Aucun emplacement de module n'est disponible dans le bâtiment actuel.";
                return false;
            }
            break;
        case ClanHeadquartersUpgradeKind::None:
            reason = "Aucune amélioration valide n'a été choisie.";
            return false;
    }

    const long long cost = upgradeCostCopper(player, kind);
    if (player.getInventory().getTotalCopper() < cost)
    {
        reason = "Fonds insuffisants pour cette amélioration.";
        return false;
    }
    if (!hasMaterialsFor(player, upgradeMaterials(player, kind), reason)) return false;

    reason = "Le chantier d'amélioration peut être lancé.";
    return true;
}

bool ClanHeadquartersSystem::startUpgrade(Player& player, ClanHeadquartersUpgradeKind kind, std::string& reason)
{
    if (!canStartUpgrade(player, kind, reason)) return false;

    const long long cost = upgradeCostCopper(player, kind);
    if (!player.getInventory().spendCopper(cost))
    {
        reason = "Le paiement de l'amélioration a échoué.";
        return false;
    }
    for (const ClanHeadquartersMaterialRequirement& requirement : upgradeMaterials(player, kind))
    {
        if (!player.getInventory().removeMaterialQuantityById(requirement.materialId, requirement.quantity))
        {
            player.getInventory().earnCopper(cost);
            reason = "Le chantier n'a pas pu réserver tous les matériaux. Le paiement a été rendu.";
            return false;
        }
    }

    const ClanHeadquartersStatus current = statusFor(player);
    int target = 0;
    if (kind == ClanHeadquartersUpgradeKind::BuildingExpansion) target = current.buildingLevel + 1;
    else if (kind == ClanHeadquartersUpgradeKind::ExtraBed) target = current.bedCapacity + 1;
    else if (kind == ClanHeadquartersUpgradeKind::QuestBoard) target = current.questBoardLevel + 1;
    else if (kind == ClanHeadquartersUpgradeKind::MissionChest) target = current.missionChestLevel + 1;
    else if (kind == ClanHeadquartersUpgradeKind::CommonRoomBar) target = current.commonRoomBarLevel + 1;
    else if (kind == ClanHeadquartersUpgradeKind::ComfortDecor) target = current.comfortDecorLevel + 1;
    else if (kind == ClanHeadquartersUpgradeKind::ForgeWorkshop) target = current.forgeWorkshopLevel + 1;

    const int startedDay = player.getWorldDaysElapsed();
    const int readyDay = startedDay + upgradeDurationDays(player, kind);
    std::ostringstream label;
    label << "kind=" << upgradeKindId(kind)
          << " | target=" << target
          << " | started_day=" << startedDay
          << " | ready_day=" << readyDay
          << " | city=" << current.cityId;
    player.recordCanonicalEvent("clan_hq_upgrade_jobs_started", "active", label.str(), 1);
    player.recordCanonicalEvent("clan_hq_investment", upgradeKindId(kind), upgradeDisplayName(kind), 1);
    reason = upgradeDisplayName(kind) + " : chantier lancé pour environ " + std::to_string(upgradeDurationDays(player, kind)) + " jour(s).";
    return true;
}

bool ClanHeadquartersSystem::completeUpgradeIfReady(Player& player, std::string* completionMessage)
{
    const ClanHeadquartersUpgradeStatus upgrade = upgradeStatusFor(player);
    if (!upgrade.active || upgrade.readyDay < 0 || player.getWorldDaysElapsed() < upgrade.readyDay) return false;

    std::string message;
    switch (upgrade.kind)
    {
        case ClanHeadquartersUpgradeKind::BuildingExpansion:
            player.recordCanonicalEvent(
                "clan_hq_building_expansions",
                "level_" + std::to_string(upgrade.targetLevel),
                "Agrandissement général terminé : QG niveau " + std::to_string(upgrade.targetLevel),
                1
            );
            player.recordCanonicalEvent(
                "clan_hq_modules",
                "expansion_level_" + std::to_string(upgrade.targetLevel),
                upgrade.targetLevel >= 3 ? "Nouvelle aile de clan, réserves et fondations renforcées" : "Aile supplémentaire et fondations renforcées",
                1
            );
            if (upgrade.targetLevel >= 3)
                message = "Le QG atteint le niveau 3. Le plafond passe à " + std::to_string(BuildingLevel3MaxBeds) + " lits et " + std::to_string(BuildingLevel3ModuleCapacity) + " emplacements de modules. Les niveaux 4 et 5 restent les futures phases structurelles maximales.";
            else
                message = "Le QG atteint le niveau 2. Le plafond de chambres passe à " + std::to_string(BuildingLevel2MaxBeds) + " et davantage de modules pourront être installés.";
            break;
        case ClanHeadquartersUpgradeKind::ExtraBed:
            player.recordCanonicalEvent(
                "clan_hq_extra_beds",
                "bed_" + std::to_string(upgrade.targetLevel),
                "Nouvelle chambre de recrue terminée : capacité " + std::to_string(upgrade.targetLevel),
                1
            );
            message = "Une nouvelle chambre est terminée. Le QG peut maintenant loger " + std::to_string(upgrade.targetLevel) + " recrue(s).";
            break;
        case ClanHeadquartersUpgradeKind::QuestBoard:
            player.recordCanonicalEvent(
                "clan_hq_quest_board_upgrades",
                "level_" + std::to_string(upgrade.targetLevel),
                "Tableau de quêtes du clan amélioré au niveau " + std::to_string(upgrade.targetLevel),
                1
            );
            player.recordCanonicalEvent("clan_hq_modules", "quest_board_level_2", "Tableau de quêtes renforcé : contrats E/D accessibles", 1);
            message = "Le tableau de quêtes atteint le niveau " + std::to_string(upgrade.targetLevel) + ". Il peut maintenant proposer des contrats de clan plus sérieux, jusqu'aux rangs E/D.";
            break;
        case ClanHeadquartersUpgradeKind::MissionChest:
            player.recordCanonicalEvent(
                "clan_hq_mission_chest_upgrades",
                "level_" + std::to_string(upgrade.targetLevel),
                "Caisse de missions installée au niveau " + std::to_string(upgrade.targetLevel),
                1
            );
            player.recordCanonicalEvent("clan_hq_modules", "mission_chest_level_1", "Caisse de missions et registre de versements installés", 1);
            message = "La caisse de missions est opérationnelle. Une petite part des contrats autonomes peut désormais être versée au clan, avec un plafond de " + std::to_string(MissionChestLevel1CapacityCopper) + " PC.";
            break;
        case ClanHeadquartersUpgradeKind::CommonRoomBar:
            player.recordCanonicalEvent(
                "clan_hq_common_room_bar_upgrades",
                "level_" + std::to_string(upgrade.targetLevel),
                "Bar / salle commune installé au niveau " + std::to_string(upgrade.targetLevel),
                1
            );
            player.recordCanonicalEvent("clan_hq_modules", "common_room_bar_level_1", "Bar, tables et espace commun du clan installés", 1);
            message = "Le bar / salle commune est opérationnel. Il offre un moment social et une petite récupération payante une fois par jour, sans bonus de combat permanent.";
            break;
        case ClanHeadquartersUpgradeKind::ComfortDecor:
            player.recordCanonicalEvent(
                "clan_hq_comfort_decor_upgrades",
                "level_" + std::to_string(upgrade.targetLevel),
                "Confort et décoration du QG améliorés au niveau " + std::to_string(upgrade.targetLevel),
                1
            );
            player.recordCanonicalEvent("clan_hq_furnishings", "comfort_decor_level_1", "Literie, éclairage, tables et décoration commune améliorés", 1);
            message = "Le QG gagne en confort : meilleure literie, éclairage, mobilier et décoration commune. Cela ne consomme pas un emplacement d'atelier et débloque l'organisation de fêtes de QG.";
            break;
        case ClanHeadquartersUpgradeKind::ForgeWorkshop:
            player.recordCanonicalEvent(
                "clan_hq_forge_workshop_upgrades",
                "level_" + std::to_string(upgrade.targetLevel),
                "Atelier de forge du clan installé au niveau " + std::to_string(upgrade.targetLevel),
                1
            );
            player.recordCanonicalEvent(
                "clan_hq_modules",
                "forge_workshop_level_1",
                "Foyer, enclume, établi et outils de forge de base installés",
                1
            );
            message = "L'atelier de forge niveau 1 est opérationnel. Il faut encore recruter un vrai artisan : le bâtiment seul ne sait ni réparer ni fabriquer quoi que ce soit.";
            break;
        case ClanHeadquartersUpgradeKind::None:
            return false;
    }

    player.recordCanonicalEvent("clan_hq_upgrade_jobs_completed", "active", "Chantier terminé : " + upgradeDisplayName(upgrade.kind), 1);
    if (completionMessage != nullptr) *completionMessage = message;
    return true;
}

std::vector<std::string> ClanHeadquartersSystem::eligibleRecruitNames(const Player& player)
{
    std::vector<std::string> names;
    for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
    {
        if (record.category != "pnj_recrutables_retenus" || record.count <= 0 || record.key.empty()) continue;
        if (recruitDismissed(player, record.key) || !recruitActuallyRecruited(player, record)) continue;
        if (std::find(names.begin(), names.end(), record.key) == names.end()) names.push_back(record.key);
    }
    std::sort(names.begin(), names.end());
    return names;
}

bool ClanHeadquartersSystem::isResidentRecruit(const Player& player, const std::string& recruitName)
{
    if (recruitName.empty()) return false;
    return recordCount(player, "clan_hq_members_joined", recruitName)
        > recordCount(player, "clan_hq_members_left", recruitName);
}

std::vector<std::string> ClanHeadquartersSystem::residentRecruitNames(const Player& player)
{
    std::vector<std::string> names;
    for (const std::string& name : eligibleRecruitNames(player))
    {
        if (isResidentRecruit(player, name)) names.push_back(name);
    }
    return names;
}

bool ClanHeadquartersSystem::assignRecruit(Player& player, const std::string& recruitName, std::string& reason)
{
    const ClanHeadquartersStatus status = statusFor(player);
    if (!status.built)
    {
        reason = "Le QG doit être terminé avant d'y rattacher une recrue.";
        return false;
    }
    const std::vector<std::string> eligible = eligibleRecruitNames(player);
    if (std::find(eligible.begin(), eligible.end(), recruitName) == eligible.end())
    {
        reason = "Cette recrue n'est pas un membre recruté disponible du clan.";
        return false;
    }
    if (isResidentRecruit(player, recruitName))
    {
        reason = recruitName + " est déjà rattaché au QG.";
        return false;
    }
    if (static_cast<int>(residentRecruitNames(player).size()) >= status.bedCapacity)
    {
        reason = status.bedCapacity >= status.maxBedCapacity
            ? "Tous les lits sont attribués et le bâtiment a atteint son plafond actuel. Un agrandissement sera nécessaire."
            : "Tous les lits sont attribués. Construis une chambre supplémentaire au QG.";
        return false;
    }

    player.recordCanonicalEvent("clan_hq_members_joined", recruitName, recruitName + " rejoint le QG du clan", 1);
    reason = recruitName + " possède désormais une place au QG et reste dans l'entourage du clan.";
    return true;
}

bool ClanHeadquartersSystem::removeRecruit(Player& player, const std::string& recruitName, std::string& reason)
{
    if (!isResidentRecruit(player, recruitName))
    {
        reason = "Cette recrue n'est pas actuellement rattachée au QG.";
        return false;
    }
    player.recordCanonicalEvent("clan_hq_members_left", recruitName, recruitName + " quitte sa chambre du QG sans être effacé de l'historique", 1);
    reason = recruitName + " libère sa place au QG. Son historique de clan reste conservé.";
    return true;
}

bool ClanHeadquartersSystem::collectMissionChest(Player& player, long long& collectedCopper, std::string& reason)
{
    collectedCopper = 0;
    const ClanHeadquartersStatus status = statusFor(player);
    if (!status.built || status.missionChestLevel <= 0)
    {
        reason = "Aucune caisse de missions n'est installée au QG.";
        return false;
    }
    if (player.getCurrentCityId() != status.cityId)
    {
        reason = "Il faut être dans la ville du QG pour récupérer la caisse de missions.";
        return false;
    }
    if (status.missionChestStoredCopper <= 0)
    {
        reason = "La caisse de missions est vide.";
        return false;
    }

    collectedCopper = status.missionChestStoredCopper;
    player.getInventory().earnCopper(collectedCopper);
    player.recordCanonicalEvent(
        "clan_hq_mission_chest_withdrawals",
        "funds",
        "Retrait manuel de la caisse de missions du clan",
        static_cast<int>(collectedCopper)
    );
    reason = "Tu récupères " + std::to_string(collectedCopper) + " PC de la caisse de missions.";
    return true;
}

bool ClanHeadquartersSystem::useCommonRoomBar(Player& player, int& healedHp, long long& paidCopper, std::string& reason)
{
    healedHp = 0;
    paidCopper = 0;
    const ClanHeadquartersStatus status = statusFor(player);
    if (!status.built || status.commonRoomBarLevel <= 0)
    {
        reason = "Aucun bar / salle commune n'est installé au QG.";
        return false;
    }
    if (player.getCurrentCityId() != status.cityId)
    {
        reason = "Il faut être dans la ville du QG pour profiter de la salle commune.";
        return false;
    }

    const std::string dayKey = "day_" + std::to_string(player.getWorldDaysElapsed());
    if (recordCount(player, "clan_hq_bar_visits", dayKey) > 0)
    {
        reason = "Tu as déjà profité du bar / de la salle commune aujourd'hui. Le QG n'est pas une fontaine de soins infinie.";
        return false;
    }
    if (player.getInventory().getTotalCopper() < CommonRoomBarVisitCostCopper)
    {
        reason = "Fonds insuffisants pour le repas / service du bar.";
        return false;
    }
    if (!player.getInventory().spendCopper(CommonRoomBarVisitCostCopper))
    {
        reason = "Le paiement du bar a échoué.";
        return false;
    }

    paidCopper = CommonRoomBarVisitCostCopper;
    const int missingHp = std::max(0, player.getMaxHp() - player.getHp());
    const int smallRecovery = status.comfortDecorLevel > 0
        ? std::max(5, player.getMaxHp() / 10)
        : std::max(4, player.getMaxHp() / 12);
    healedHp = std::min(missingHp, smallRecovery);
    if (healedHp > 0) player.heal(healedHp);

    const std::vector<std::string> residents = residentRecruitNames(player);
    std::string socialLabel = "Moment de détente dans la salle commune";
    if (!residents.empty())
    {
        const std::size_t index = stableHash(dayKey + ":bar_social") % residents.size();
        socialLabel += " avec " + residents[index];
    }
    player.recordCanonicalEvent("clan_hq_bar_visits", dayKey, socialLabel, 1);
    player.recordCanonicalEvent("clan_hq_social_moments", "bar", socialLabel, 1);

    if (healedHp > 0)
    {
        reason = "Un repas et un moment au calme rendent " + std::to_string(healedHp) + " PV. Le reste est surtout du confort et de la vie de clan.";
    }
    else
    {
        reason = "Tu étais déjà en pleine forme : aucun PV gagné, mais le passage compte comme moment social du clan.";
    }
    return true;
}

long long ClanHeadquartersSystem::celebrationCostCopper(const Player& player)
{
    const ClanHeadquartersStatus status = statusFor(player);
    const long long discount = status.commonRoomBarLevel > 0 ? CelebrationBarDiscountCopper : 0;
    return std::max<long long>(0, CelebrationBaseCostCopper - discount);
}

bool ClanHeadquartersSystem::canOrganizeCelebration(const Player& player, std::string& reason)
{
    const ClanHeadquartersStatus status = statusFor(player);
    if (!status.built)
    {
        reason = "Le QG doit d'abord être construit.";
        return false;
    }
    if (player.getCurrentCityId() != status.cityId)
    {
        reason = "Il faut être dans la ville du QG pour organiser une fête de clan.";
        return false;
    }
    if (status.comfortDecorLevel <= 0)
    {
        reason = "Améliore d'abord le confort / la décoration du QG : il faut un espace réellement accueillant pour organiser une fête.";
        return false;
    }
    const std::vector<std::string> residents = residentRecruitNames(player);
    if (residents.size() < 2)
    {
        reason = "Il faut au moins deux membres résidents au QG pour organiser une vraie fête de clan.";
        return false;
    }
    const PlayerJournalRecord* previous = findRecord(player, "clan_hq_festivals", "headquarters_party");
    if (previous != nullptr)
    {
        const int nextDay = previous->lastDay + CelebrationCooldownDays;
        if (player.getWorldDaysElapsed() < nextDay)
        {
            reason = "Le clan vient déjà d'organiser une fête. Prochaine grande soirée possible vers le jour " + std::to_string(nextDay) + ".";
            return false;
        }
    }
    const long long cost = celebrationCostCopper(player);
    if (player.getInventory().getTotalCopper() < cost)
    {
        reason = "Fonds insuffisants pour nourriture, boissons, musique et organisation de la fête.";
        return false;
    }
    reason = "La fête peut être organisée.";
    return true;
}

bool ClanHeadquartersSystem::organizeCelebration(Player& player, ClanHeadquartersCelebrationReport& report, std::string& reason)
{
    report = ClanHeadquartersCelebrationReport{};
    if (!canOrganizeCelebration(player, reason)) return false;

    const long long cost = celebrationCostCopper(player);
    if (!player.getInventory().spendCopper(cost))
    {
        reason = "Le paiement de la fête a échoué.";
        return false;
    }

    report.paidCopper = cost;
    report.participantNames = residentRecruitNames(player);
    report.participantCount = static_cast<int>(report.participantNames.size());
    report.moraleChargesGranted = report.participantCount;
    report.clanReputationGranted = std::clamp(1 + report.participantCount / 3, 1, 3);

    std::ostringstream label;
    label << "Fête du QG organisée au jour " << player.getWorldDaysElapsed()
          << " | participants=" << report.participantCount
          << " | coût=" << cost;
    player.recordCanonicalEvent("clan_hq_festivals", "headquarters_party", label.str(), 1);
    player.recordCanonicalEvent("clan_hq_social_moments", "festival", "Grande soirée organisée au QG avec les membres résidents", 1);
    player.recordCanonicalEvent("reputation_clan", "fetes_qg", "Vie communautaire et cohésion du clan", report.clanReputationGranted);

    for (const std::string& name : report.participantNames)
    {
        player.recordCanonicalEvent("clan_hq_festival_participants", name, name + " participe à la fête du QG", 1);
        player.recordCanonicalEvent("clan_hq_festive_morale", name, "Motivation sociale pour la prochaine mission autonome", 1);
    }

    report.lines.push_back("La fête rassemble " + std::to_string(report.participantCount) + " membre(s) résident(s) du clan.");
    report.lines.push_back("Coût d'organisation : " + std::to_string(cost) + " PC. Aucun revenu ni buff de combat permanent n'est créé.");
    report.lines.push_back("Chaque participant gagne une petite motivation sociale consommée lors de sa prochaine mission autonome (+2 points de pourcentage d'XP, toujours plafonnés à 25%).");
    report.lines.push_back("Réputation de clan : +" + std::to_string(report.clanReputationGranted) + ". Prochaine grande fête possible dans environ " + std::to_string(CelebrationCooldownDays) + " jours.");
    if (statusFor(player).commonRoomBarLevel > 0)
        report.lines.push_back("Le bar déjà installé réduit légèrement le coût logistique de la soirée.");

    reason = "La fête du QG s'est bien déroulée.";
    return true;
}

ClanHeadquartersPassiveReport ClanHeadquartersSystem::processPassiveActivity(Player& player)
{
    ClanHeadquartersPassiveReport report;
    if (!statusFor(player).built)
    {
        report.lines.push_back("Aucun QG terminé : aucune activité de clan autonome à traiter.");
        return report;
    }

    const PlayerJournalRecord* clock = findRecord(player, "clan_hq_passive_clock", "last_check");
    if (clock == nullptr)
    {
        player.recordCanonicalEvent("clan_hq_passive_clock", "last_check", "Horloge d'activité du QG initialisée", 1);
        report.lines.push_back("Horloge du QG initialisée. Les prochaines activités seront calculées avec le temps du monde.");
        return report;
    }

    const int currentDay = player.getWorldDaysElapsed();
    report.elapsedDays = std::max(0, currentDay - clock->lastDay);
    report.missionCycles = std::min(6, report.elapsedDays / 3);
    if (report.missionCycles <= 0)
    {
        report.lines.push_back("Pas assez de temps écoulé pour une nouvelle mission autonome du clan (environ une fenêtre tous les 3 jours). ");
        return report;
    }

    const ClanHeadquartersStatus status = statusFor(player);
    const int boardMaxRankIndex = status.questBoardLevel >= 2 ? 2 : 1; // level 1: F/E, level 2: F/E/D.
    std::vector<std::string> availableResidents;
    for (const std::string& name : residentRecruitNames(player))
    {
        if (!recruitEquipped(player, name) && !recruitUnavailable(player, name)) availableResidents.push_back(name);
    }
    const bool grouped = availableResidents.size() >= 2;
    int groupMaximumRankIndex = -1;
    if (grouped)
    {
        std::vector<int> availableRanks;
        availableRanks.reserve(availableResidents.size());
        for (const std::string& name : availableResidents) availableRanks.push_back(missionProfileFor(player, name).rankIndex);
        std::vector<int> allyRanks(availableRanks.begin() + 1, availableRanks.end());
        groupMaximumRankIndex = GuildQuestAccessPolicy::groupMaximumQuestRankIndex(availableRanks.front(), allyRanks);
    }

    for (const std::string& name : residentRecruitNames(player))
    {
        if (recruitEquipped(player, name) || recruitUnavailable(player, name))
        {
            report.lines.push_back("- " + name + " n'est pas envoyé en mission autonome : actuellement avec le joueur ou indisponible.");
            continue;
        }

        const RecruitMissionProfile profile = missionProfileFor(player, name);
        const int maxAllowedByRecruit = grouped ? groupMaximumRankIndex : profile.rankIndex;
        const int missionRankIndex = std::min(boardMaxRankIndex, maxAllowedByRecruit);
        const std::string missionRank = recruitRankName(missionRankIndex);
        int totalXp = 0;
        long long recruitGeneratedCopper = 0;
        int minPercent = 100;
        int maxPercent = 0;

        const int festiveGranted = recordCount(player, "clan_hq_festive_morale", name);
        const int festiveConsumed = recordCount(player, "clan_hq_festive_morale_consumed", name);
        bool festiveChargeAvailable = festiveGranted > festiveConsumed;
        bool festiveChargeUsed = false;
        for (int cycle = 0; cycle < report.missionCycles; ++cycle)
        {
            int percent = passiveExperiencePercent(name, currentDay, cycle);
            if (festiveChargeAvailable && !festiveChargeUsed)
            {
                percent = std::min(25, percent + 2);
                festiveChargeUsed = true;
            }
            minPercent = std::min(minPercent, percent);
            maxPercent = std::max(maxPercent, percent);
            const int normalXp = typicalActiveQuestExperience(missionRankIndex, profile.estimatedLevel);
            totalXp += std::max(1, normalXp * percent / 100);
            recruitGeneratedCopper += passiveMissionCopper(missionRankIndex, profile.estimatedLevel);
        }
        if (festiveChargeUsed)
        {
            player.recordCanonicalEvent("clan_hq_festive_morale_consumed", name, "Motivation de fête consommée sur une mission autonome", 1);
        }

        player.recordCanonicalEvent("clan_hq_missions_completed", name, name + " accomplit des missions du tableau du clan", report.missionCycles);
        player.recordCanonicalEvent("xp_recrues", name, name + " gagne 15-25% de l'XP normale via les missions autonomes du QG", totalXp);
        report.missionsCompleted += report.missionCycles;
        report.totalRecruitXp += totalXp;
        report.generatedMissionCopper += recruitGeneratedCopper;
        report.lines.push_back(
            "- " + name + " : " + std::to_string(report.missionCycles) + " contrat(s) rang " + missionRank
            + (availableResidents.size() >= 5 ? " (plafond calculé sur la moyenne du groupe)"
                : (grouped && missionRankIndex > profile.rankIndex ? " (un rang au-dessus grâce au groupe)" : ""))
            + ", +" + std::to_string(totalXp) + " XP (" + std::to_string(minPercent) + "-" + std::to_string(maxPercent)
            + "% d'une quête active équivalente)."
        );
    }

    if (report.missionsCompleted > 0)
    {
        const int divisor = status.questBoardLevel >= 2 ? 2 : 3;
        const int clanReputation = std::max(1, report.missionsCompleted / divisor);
        player.recordCanonicalEvent("reputation_clan", "missions_qg", "Missions autonomes correctement terminées par le QG", clanReputation);

        if (status.missionChestLevel > 0 && report.generatedMissionCopper > 0)
        {
            const long long freeCapacity = std::max<long long>(0, status.missionChestCapacityCopper - status.missionChestStoredCopper);
            report.storedMissionCopper = std::min(freeCapacity, report.generatedMissionCopper);
            report.discardedMissionCopper = std::max<long long>(0, report.generatedMissionCopper - report.storedMissionCopper);
            if (report.storedMissionCopper > 0)
            {
                player.recordCanonicalEvent(
                    "clan_hq_mission_chest_deposits",
                    "funds",
                    "Petite part des contrats autonomes versée à la caisse du clan",
                    static_cast<int>(report.storedMissionCopper)
                );
            }
            report.lines.push_back(
                "Caisse de missions : +" + std::to_string(report.storedMissionCopper) + " PC stockés sur "
                + std::to_string(report.generatedMissionCopper) + " PC générés pour le clan."
            );
            if (report.discardedMissionCopper > 0)
            {
                report.lines.push_back(
                    "La caisse est pleine : " + std::to_string(report.discardedMissionCopper)
                    + " PC ne sont pas stockés. Les aventuriers restent libres de privilégier des contrats de guilde plutôt que de travailler gratuitement pour le clan."
                );
            }
        }
        else if (status.missionChestLevel <= 0)
        {
            report.lines.push_back("Sans caisse de missions, les recrues couvrent leurs propres frais : aucun argent autonome n'arrive directement dans la poche du joueur.");
        }
    }
    else
    {
        report.lines.push_back("Aucun membre disponible n'a effectué de mission autonome pendant cette période.");
    }

    player.recordCanonicalEvent("clan_hq_passive_clock", "last_check", "Activité autonome du QG traitée", 1);
    return report;
}
