#include "combat/group/TeamEmploymentMissionSystem.hpp"
#include "combat/group/TeamEmploymentSystem.hpp"
#include "class_system/ClassCatalog.hpp"
#include "entity/Player.hpp"
#include "quest/Quest.hpp"

#include <cassert>

int main()
{
    Player player("Trexof", ClassCatalog::createClassByName("Guerrier"));
    assert(TeamEmploymentMissionSystem::buildCurrentMission(player).id.empty());
    assert(!TeamEmploymentMissionSystem::assignCurrentMission(player));

    assert(TeamEmploymentSystem::startExternalContract(
        player,
        "garde_est",
        "Capitaine Maelis",
        "garde_de_lest",
        "Garde de l'Est",
        "éclaireur",
        12,
        7,
        -1
    ));

    Quest offer = TeamEmploymentMissionSystem::buildCurrentMission(player);
    assert(!offer.id.empty());
    assert(offer.client == "Capitaine Maelis");
    assert(offer.origin.find("Garde de l'Est") != std::string::npos);
    assert(offer.objectiveType == "combat");
    assert(offer.target >= 2 && offer.target <= 6);
    assert(offer.accepted);
    assert(offer.expiresAtDay == player.getWorldDaysElapsed() + 2);

    assert(TeamEmploymentMissionSystem::assignCurrentMission(player));
    assert(!TeamEmploymentMissionSystem::assignCurrentMission(player));
    assert(TeamEmploymentMissionSystem::hasOpenEmployerMission(player));
    const std::string missionId = TeamEmploymentMissionSystem::openEmployerMissionId(player);
    assert(!missionId.empty());

    assert(player.getQuestLog().completeQuest(missionId));
    assert(player.getQuestLog().turnInQuest(missionId));
    assert(TeamEmploymentMissionSystem::synchronizeMissionHistory(player) == 1);
    assert(TeamEmploymentMissionSystem::synchronizeMissionHistory(player) == 0);
    assert(player.getCanonicalJournalCategoryTotal("team_employment_missions_completed") == 1);

    player.advanceWorldDays(1);
    assert(TeamEmploymentMissionSystem::assignCurrentMission(player));
    const std::string secondMissionId = TeamEmploymentMissionSystem::openEmployerMissionId(player);
    assert(!secondMissionId.empty() && secondMissionId != missionId);
    for (Quest& quest : player.getQuestLog().getQuests())
    {
        if (quest.id == secondMissionId)
        {
            quest.failed = true;
            quest.failureReason = "ordre non accompli";
        }
    }
    assert(TeamEmploymentMissionSystem::synchronizeMissionHistory(player) == 1);
    assert(player.getCanonicalJournalCategoryTotal("team_employment_missions_failed") == 1);

    Player leaving("Elara", ClassCatalog::createClassByName("Guerrier"));
    assert(TeamEmploymentSystem::startExternalContract(
        leaving, "escorte_courte", "Capitaine Sera", "escorte", "Escorte de Sera", "garde", 8, 7, -1));
    assert(TeamEmploymentMissionSystem::assignCurrentMission(leaving));
    const std::string leavingMissionId = TeamEmploymentMissionSystem::openEmployerMissionId(leaving);
    assert(!leavingMissionId.empty());
    assert(TeamEmploymentSystem::resign(leaving, "depart_volontaire"));
    bool interruptedMissionFound = false;
    for (const Quest& quest : leaving.getQuestLog().getQuests())
    {
        if (quest.id == leavingMissionId)
        {
            interruptedMissionFound = true;
            assert(quest.failed);
            assert(quest.failureReason.find("contrat") != std::string::npos);
        }
    }
    assert(interruptedMissionFound);
    assert(leaving.getCanonicalJournalCategoryTotal("team_employment_missions_failed") == 1);

    return 0;
}
