#include "world/clan/ClanHeadquartersSystem.hpp"

#include "entity/Player.hpp"
#include "item/material/MaterialCatalog.hpp"

#include <cassert>
#include <string>
#include <vector>

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

    void addConstructionMaterials(Player& player)
    {
        for (const ClanHeadquartersMaterialRequirement& requirement : ClanHeadquartersSystem::baseConstructionMaterials())
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
    player.recordCanonicalEvent("tests_groupe_guilde_reussis", "base", "Test de groupe validé", 1);
    player.recordCanonicalEvent("clan_nom", "nom_actuel", "Les Lucioles", 1);
    player.getInventory().earnCopper(ClanHeadquartersSystem::BaseConstructionCostCopper + 5000);
    addConstructionMaterials(player);

    std::string reason;
    assert(ClanHeadquartersSystem::canStartConstruction(player, reason));
    assert(ClanHeadquartersSystem::startConstruction(player, reason));

    ClanHeadquartersStatus status = ClanHeadquartersSystem::statusFor(player);
    assert(status.constructionStarted);
    assert(!status.built);
    assert(status.constructionReadyDay == player.getWorldDaysElapsed() + ClanHeadquartersSystem::BaseConstructionDays);
    assert(!ClanHeadquartersSystem::startConstruction(player, reason)); // unique chantier/QG

    player.advanceWorldDays(ClanHeadquartersSystem::BaseConstructionDays - 1);
    assert(!ClanHeadquartersSystem::completeConstructionIfReady(player));
    player.advanceWorldDays(1);
    assert(ClanHeadquartersSystem::completeConstructionIfReady(player));

    status = ClanHeadquartersSystem::statusFor(player);
    assert(status.built);
    assert(status.buildingLevel == 1);
    assert(status.bedCapacity == 2);
    assert(status.questBoardLevel == 1);
    assert(status.counterAvailable);

    addRecruit(player, "Aline");
    addRecruit(player, "Boros");
    addRecruit(player, "Celia");

    assert(ClanHeadquartersSystem::assignRecruit(player, "Aline", reason));
    assert(ClanHeadquartersSystem::assignRecruit(player, "Boros", reason));
    assert(!ClanHeadquartersSystem::assignRecruit(player, "Celia", reason)); // two base beds only
    assert(ClanHeadquartersSystem::residentRecruitNames(player).size() == 2);

    player.advanceWorldDays(3);
    ClanHeadquartersPassiveReport report = ClanHeadquartersSystem::processPassiveActivity(player);
    assert(report.missionCycles == 1);
    assert(report.missionsCompleted == 2);
    assert(report.totalRecruitXp > 0);
    assert(count(player, "xp_recrues", "Aline") > 0);
    assert(count(player, "xp_recrues", "Boros") > 0);

    // Active adventurers do not also farm an autonomous mission at the same time.
    const int alineXpBefore = count(player, "xp_recrues", "Aline");
    const int borosXpBefore = count(player, "xp_recrues", "Boros");
    player.recordCanonicalEvent("recrues_equipees_toggle", "Aline", "Aline équipé", 1);
    player.advanceWorldDays(3);
    report = ClanHeadquartersSystem::processPassiveActivity(player);
    assert(count(player, "xp_recrues", "Aline") == alineXpBefore);
    assert(count(player, "xp_recrues", "Boros") > borosXpBefore);

    assert(ClanHeadquartersSystem::removeRecruit(player, "Boros", reason));
    assert(!ClanHeadquartersSystem::isResidentRecruit(player, "Boros"));
    assert(ClanHeadquartersSystem::assignRecruit(player, "Celia", reason));
    assert(ClanHeadquartersSystem::residentRecruitNames(player).size() == 2);

    return 0;
}
