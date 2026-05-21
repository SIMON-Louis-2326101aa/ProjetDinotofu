// EN: QuestLog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: QuestLog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu.
// Description: Implements the player's quest journal.

#include "quest/QuestLog.hpp"

#include <algorithm>
#include <cctype>

namespace
{
    std::string normalizeQuestText(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        return value;
    }

    // EN: questTargetMatchesEncounter declares or implements a focused behavior used by this module.
    // FR: questTargetMatchesEncounter déclare ou implémente un comportement précis utilisé par ce module.
    bool questTargetMatchesEncounter(const Quest& quest, const std::string& encounterFamily)
    {
        std::string target = normalizeQuestText(quest.targetFamily);
        std::string encounter = normalizeQuestText(encounterFamily);

        if (encounter.empty())
        {
            return true;
        }

        if (target.empty() || target == "générale" || target == "generale")
        {
            return true;
        }

        return target.find(encounter) != std::string::npos
            || encounter.find(target) != std::string::npos
            || (target.find("menace") != std::string::npos && encounter.find("menace") != std::string::npos)
            || (target.find("créature") != std::string::npos && encounter.find("créature") != std::string::npos)
            || (target.find("creature") != std::string::npos && encounter.find("creature") != std::string::npos)
            || (target.find("élite") != std::string::npos && encounter.find("mini-boss") != std::string::npos)
            || (target.find("elite") != std::string::npos && encounter.find("mini-boss") != std::string::npos);
    }
}

// EN: getQuests declares or implements a focused behavior used by this module.
// FR: getQuests déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<Quest>& QuestLog::getQuests() const
{
    return quests;
}

std::vector<Quest>& QuestLog::getQuests()
{
    return quests;
}

// EN: hasQuest declares or implements a focused behavior used by this module.
// FR: hasQuest déclare ou implémente un comportement précis utilisé par ce module.
bool QuestLog::hasQuest(const std::string& questId) const
{
    for (const Quest& quest : quests)
    {
        if (quest.id == questId && !quest.turnedIn)
        {
            return true;
        }
    }

    return false;
}

// EN: getActiveGuildQuestCount declares or implements a focused behavior used by this module.
// FR: getActiveGuildQuestCount déclare ou implémente un comportement précis utilisé par ce module.
int QuestLog::getActiveGuildQuestCount() const
{
    int count = 0;

    for (const Quest& quest : quests)
    {
        if (quest.guildQuest && quest.accepted && !quest.turnedIn)
        {
            count++;
        }
    }

    return count;
}


// EN: getActivePersonalQuestCountForClient declares or implements a focused behavior used by this module.
// FR: getActivePersonalQuestCountForClient déclare ou implémente un comportement précis utilisé par ce module.
int QuestLog::getActivePersonalQuestCountForClient(const std::string& client) const
{
    int count = 0;

    for (const Quest& quest : quests)
    {
        if (!quest.guildQuest && quest.client == client && quest.accepted && !quest.turnedIn)
        {
            count++;
        }
    }

    return count;
}

// EN: canAcceptGuildQuest declares or implements a focused behavior used by this module.
// FR: canAcceptGuildQuest déclare ou implémente un comportement précis utilisé par ce module.
bool QuestLog::canAcceptGuildQuest() const
{
    return getActiveGuildQuestCount() < 3;
}

// EN: canAcceptPersonalQuestForClient declares or implements a focused behavior used by this module.
// FR: canAcceptPersonalQuestForClient déclare ou implémente un comportement précis utilisé par ce module.
bool QuestLog::canAcceptPersonalQuestForClient(const std::string& client) const
{
    return getActivePersonalQuestCountForClient(client) < 2;
}

// EN: addQuest declares or implements a focused behavior used by this module.
// FR: addQuest déclare ou implémente un comportement précis utilisé par ce module.
bool QuestLog::addQuest(const Quest& quest)
{
    if (hasQuest(quest.id))
    {
        return false;
    }

    if (quest.guildQuest && !canAcceptGuildQuest())
    {
        return false;
    }

    if (!quest.guildQuest && !canAcceptPersonalQuestForClient(quest.client))
    {
        return false;
    }

    quests.push_back(quest);
    return true;
}

// EN: progressQuest declares or implements a focused behavior used by this module.
// FR: progressQuest déclare ou implémente un comportement précis utilisé par ce module.
bool QuestLog::progressQuest(const std::string& questId, int amount)
{
    for (Quest& quest : quests)
    {
        if (quest.id == questId && quest.accepted && !quest.turnedIn)
        {
            quest.progress += amount;

            if (quest.progress >= quest.target)
            {
                quest.progress = quest.target;
                quest.completed = true;
            }

            return true;
        }
    }

    return false;
}

// EN: completeQuest declares or implements a focused behavior used by this module.
// FR: completeQuest déclare ou implémente un comportement précis utilisé par ce module.
bool QuestLog::completeQuest(const std::string& questId)
{
    for (Quest& quest : quests)
    {
        if (quest.id == questId && quest.accepted && !quest.turnedIn)
        {
            quest.progress = quest.target;
            quest.completed = true;
            return true;
        }
    }

    return false;
}

// EN: turnInQuest declares or implements a focused behavior used by this module.
// FR: turnInQuest déclare ou implémente un comportement précis utilisé par ce module.
bool QuestLog::turnInQuest(const std::string& questId)
{
    for (Quest& quest : quests)
    {
        if (quest.id == questId && quest.completed && !quest.turnedIn)
        {
            quest.turnedIn = true;
            return true;
        }
    }

    return false;
}

// EN: progressCombatQuests declares or implements a focused behavior used by this module.
// FR: progressCombatQuests déclare ou implémente un comportement précis utilisé par ce module.
int QuestLog::progressCombatQuests(int defeatedEnemyCount)
{
    return progressCombatQuestsByFamily(defeatedEnemyCount, "");
}

// EN: progressCombatQuestsByFamily declares or implements a focused behavior used by this module.
// FR: progressCombatQuestsByFamily déclare ou implémente un comportement précis utilisé par ce module.
int QuestLog::progressCombatQuestsByFamily(int defeatedEnemyCount, const std::string& encounterFamily)
{
    if (defeatedEnemyCount <= 0)
    {
        return 0;
    }

    int updated = 0;

    for (Quest& quest : quests)
    {
        if (!quest.accepted || quest.completed || quest.turnedIn)
        {
            continue;
        }

        // Les quêtes de livraison se valident avec les ressources, pas avec un compteur simulé.
        if (!quest.requiredMaterialId.empty() && quest.requiredMaterialQuantity > 0)
        {
            continue;
        }

        if (quest.objectiveType != "combat" && quest.objectiveType != "exploration" && quest.objectiveType != "bestiaire")
        {
            continue;
        }

        if (!questTargetMatchesEncounter(quest, encounterFamily))
        {
            continue;
        }

        quest.progress += defeatedEnemyCount;

        if (quest.progress >= quest.target)
        {
            quest.progress = quest.target;
            quest.completed = true;
        }

        updated++;
    }

    return updated;
}

// EN: refreshMaterialDeliveryQuests declares or implements a focused behavior used by this module.
// FR: refreshMaterialDeliveryQuests déclare ou implémente un comportement précis utilisé par ce module.
int QuestLog::refreshMaterialDeliveryQuests(const Inventory& inventory)
{
    int updated = 0;

    for (Quest& quest : quests)
    {
        if (!quest.accepted || quest.completed || quest.turnedIn)
        {
            continue;
        }

        if (quest.requiredMaterialId.empty() || quest.requiredMaterialQuantity <= 0)
        {
            continue;
        }

        int requiredQualityPoints = quest.requiredMaterialQuantity * 2;

        if (inventory.countMaterialQualityPointsById(quest.requiredMaterialId) >= requiredQualityPoints)
        {
            quest.progress = quest.target;
            quest.completed = true;
            updated++;
        }
    }

    return updated;
}

// EN: hasTurnInReadyQuestForClient declares or implements a focused behavior used by this module.
// FR: hasTurnInReadyQuestForClient déclare ou implémente un comportement précis utilisé par ce module.
bool QuestLog::hasTurnInReadyQuestForClient(const std::string& client) const
{
    for (const Quest& quest : quests)
    {
        if (quest.client == client && quest.completed && !quest.turnedIn)
        {
            return true;
        }
    }

    return false;
}

// EN: clear declares or implements a focused behavior used by this module.
// FR: clear déclare ou implémente un comportement précis utilisé par ce module.
void QuestLog::clear()
{
    quests.clear();
}
