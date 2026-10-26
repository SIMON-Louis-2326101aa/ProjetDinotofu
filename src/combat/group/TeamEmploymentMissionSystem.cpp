#include "combat/group/TeamEmploymentMissionSystem.hpp"

#include "combat/group/TeamEmploymentSystem.hpp"
#include "combat/group/EmployerTeamDisciplineSystem.hpp"
#include "entity/Player.hpp"
#include "quest/Quest.hpp"
#include "quest/QuestLog.hpp"

#include <algorithm>

namespace
{
    int canonicalRecordCount(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key)
            {
                return record.count;
            }
        }
        return 0;
    }

    std::string missionPrefix(const TeamEmploymentContract& contract)
    {
        return "team_job_" + (contract.contractId.empty() ? std::string("external") : contract.contractId) + "_";
    }

    std::string rankForLevel(int level)
    {
        if (level >= 55) return "S";
        if (level >= 38) return "A";
        if (level >= 24) return "B";
        if (level >= 14) return "C";
        if (level >= 7) return "D";
        if (level >= 3) return "E";
        return "F";
    }
}

Quest TeamEmploymentMissionSystem::buildCurrentMission(const Player& player)
{
    Quest quest;
    const TeamEmploymentContract contract = TeamEmploymentSystem::contractFor(player);
    if (!contract.active)
    {
        return quest;
    }

    const int day = std::max(0, player.getWorldDaysElapsed());
    const int level = std::max(1, player.getLevel());
    const int target = std::clamp(2 + level / 18, 2, 6);
    const std::string organization = contract.organizationName.empty() ? std::string("l'équipe") : contract.organizationName;

    quest.id = missionPrefix(contract) + "day_" + std::to_string(day);
    quest.rank = rankForLevel(level);
    quest.title = "Ordre de terrain — " + organization;
    quest.origin = "Équipe employeuse — " + organization;
    quest.client = contract.leaderName.empty() ? std::string("Chef d'équipe") : contract.leaderName;
    quest.location = player.getCurrentCityId();
    quest.objective = "Remporter " + std::to_string(target) + " affrontement(s) réel(s) pendant le service de l'équipe.";
    quest.objectiveType = "combat";
    quest.targetFamily.clear();
    quest.rewardExperience = std::max(4, 5 + level * 2);
    quest.rewardGold = std::clamp(1 + level / 25, 1, 5);
    quest.rewardNote = "Prime de mission distincte du salaire contractuel.";
    quest.progress = 0;
    quest.target = target;
    quest.availableFromDay = day;
    quest.expiresAtDay = day + 2;
    quest.accepted = true;
    quest.guildQuest = false;
    quest.guildChallenge = false;
    return quest;
}

bool TeamEmploymentMissionSystem::isEmployerMissionForCurrentContract(const Player& player, const Quest& quest)
{
    const TeamEmploymentContract contract = TeamEmploymentSystem::contractFor(player);
    if (!contract.active || quest.id.empty())
    {
        return false;
    }
    return quest.id.rfind(missionPrefix(contract), 0) == 0;
}

bool TeamEmploymentMissionSystem::hasOpenEmployerMission(const Player& player)
{
    for (const Quest& quest : player.getQuestLog().getQuests())
    {
        if (isEmployerMissionForCurrentContract(player, quest)
            && quest.accepted && !quest.turnedIn && !quest.failed)
        {
            return true;
        }
    }
    return false;
}

std::string TeamEmploymentMissionSystem::openEmployerMissionId(const Player& player)
{
    for (const Quest& quest : player.getQuestLog().getQuests())
    {
        if (isEmployerMissionForCurrentContract(player, quest)
            && quest.accepted && !quest.turnedIn && !quest.failed)
        {
            return quest.id;
        }
    }
    return "";
}

bool TeamEmploymentMissionSystem::assignCurrentMission(Player& player)
{
    if (!TeamEmploymentSystem::hasActiveContract(player) || hasOpenEmployerMission(player))
    {
        return false;
    }

    Quest quest = buildCurrentMission(player);
    if (quest.id.empty())
    {
        return false;
    }

    if (!player.getQuestLog().addQuest(quest))
    {
        return false;
    }

    const TeamEmploymentContract contract = TeamEmploymentSystem::contractFor(player);
    player.recordCanonicalEvent(
        "team_employment_missions_assigned",
        quest.id,
        (contract.organizationName.empty() ? std::string("Employeur") : contract.organizationName)
            + " | ordre reçu : " + quest.title,
        1
    );
    return true;
}


bool TeamEmploymentMissionSystem::progressServiceCombat(Player& player, int amount)
{
    if (amount <= 0) return false;
    for (Quest& quest : player.getQuestLog().getQuests())
    {
        if (!isEmployerMissionForCurrentContract(player, quest)
            || !quest.accepted || quest.turnedIn || quest.failed || quest.completed)
        {
            continue;
        }
        if (quest.objectiveType != "combat") continue;
        quest.progress = std::min(quest.target, quest.progress + amount);
        if (quest.progress >= quest.target) quest.completed = true;
        return true;
    }
    return false;
}

int TeamEmploymentMissionSystem::synchronizeMissionHistory(Player& player)
{
    int synchronized = 0;
    for (Quest& quest : player.getQuestLog().getQuests())
    {
        if (!isEmployerMissionForCurrentContract(player, quest))
        {
            continue;
        }

        if (quest.accepted && !quest.completed && !quest.turnedIn && !quest.failed
            && quest.expiresAtDay >= 0 && player.getWorldDaysElapsed() > quest.expiresAtDay)
        {
            quest.failed = true;
            quest.failureReason = "Ordre employeur expiré : la mission devait être accomplie avant la fin du jour " + std::to_string(quest.expiresAtDay) + ".";
        }

        if (quest.turnedIn && canonicalRecordCount(player, "team_employment_missions_completed", quest.id) <= 0)
        {
            TeamEmploymentSystem::recordEmployerMissionCompleted(player, quest.id, quest.title);
            ++synchronized;
        }
        else if (quest.failed && canonicalRecordCount(player, "team_employment_missions_failed", quest.id) <= 0)
        {
            TeamEmploymentSystem::recordEmployerMissionFailed(player, quest.id, quest.title);
            ++synchronized;
        }
    }
    if (TeamEmploymentSystem::hasActiveContract(player)
        && EmployerTeamDisciplineSystem::dismissalRecommended(player))
    {
        const int retention = EmployerTeamDisciplineSystem::recommendedSalaryRetentionPercent(player);
        TeamEmploymentSystem::dismissForDiscipline(
            player,
            "fautes_professionnelles_repetees",
            retention
        );
        ++synchronized;
    }

    return synchronized;
}
