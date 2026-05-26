// EN: QuestLog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: QuestLog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu.
// Description: Implements the player's quest journal.

#include "quest/QuestLog.hpp"
#include "quest/QuestCatalog.hpp"

#include <algorithm>
#include <cctype>
#include <vector>

namespace
{
    std::string normalizeQuestText(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        return value;
    }


    std::vector<std::string> monsterQuestFamilies(const Monster& monster)
    {
        std::vector<std::string> families;

        families.push_back("Créatures locales");
        families.push_back(monster.getRaceText());
        families.push_back(monster.getType());

        if (monster.isEvolved())
        {
            families.push_back("Créature évoluée");
            families.push_back("Mini-boss / menace évoluée");
        }

        if (monster.isElite())
        {
            families.push_back("Élite / menace");
            families.push_back("Menace avancée");
        }

        switch (monster.getRace())
        {
            case Race::Gobelin:
            case Race::Hobgobelin:
            case Race::Orc:
            case Race::Humain:
            case Race::SemiHumain:
            case Race::Elfe:
            case Race::ElfeNoir:
            case Race::Nain:
            case Race::Gnome:
            case Race::Halfelin:
            case Race::Tieffelin:
            case Race::Aasimar:
            case Race::Kitsune:
            case Race::Fee:
            case Race::SemiDragon:
                families.push_back("Humanoïdes / embuscades");
                break;
            case Race::Bete:
            case Race::Plante:
            case Race::Insectoide:
            case Race::Slime:
            case Race::Draconide:
                families.push_back("Créatures faibles");
                families.push_back("Créatures naturelles");
                break;
            case Race::MortVivant:
                families.push_back("Morts-vivants et reliques");
                families.push_back("Ruines effondrées");
                break;
            case Race::Dragon:
            case Race::Demon:
            case Race::Aberration:
            case Race::AnomalieArcanique:
                families.push_back("Menace avancée");
                break;
            case Race::Elementaire:
            case Race::Esprit:
                families.push_back("Bestiaire / observation");
                families.push_back("Variation d'énergie");
                break;
            default:
                break;
        }

        std::string name = normalizeQuestText(monster.getName());
        if (name.find("loup") != std::string::npos || name.find("sanglier") != std::string::npos || name.find("ours") != std::string::npos)
        {
            families.push_back("Forêt ancienne / créatures naturelles");
            families.push_back("Plaine sauvage / créatures locales");
        }
        if (name.find("slime") != std::string::npos)
        {
            families.push_back("Marais trouble / slimes et noyés");
        }
        if (name.find("gobelin") != std::string::npos || name.find("bandit") != std::string::npos || name.find("pillard") != std::string::npos)
        {
            families.push_back("Route commerciale / humanoïdes et embuscades");
        }
        if (name.find("squelette") != std::string::npos || name.find("goule") != std::string::npos || name.find("revenant") != std::string::npos)
        {
            families.push_back("Ruines effondrées / morts-vivants et reliques");
        }
        if (name.find("yéti") != std::string::npos || name.find("roche") != std::string::npos || name.find("givre") != std::string::npos)
        {
            families.push_back("Montagne froide / bêtes de givre");
        }

        return families;
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

QuestLog::QuestLog()
    : guildBoardCreatedAtCombat(-1),
      guildBoardTargetSize(0),
      guildBoardPendingReplacements(0),
      guildBoardReplacementDueAtCombat(-1)
{
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

        if (quest.objectiveType != "combat" && quest.objectiveType != "bestiaire")
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


int QuestLog::progressCombatQuestsForMonster(const Monster& monster, int amount)
{
    if (amount <= 0)
    {
        return 0;
    }

    int updated = 0;
    std::vector<std::string> families = monsterQuestFamilies(monster);

    for (Quest& quest : quests)
    {
        if (!quest.accepted || quest.completed || quest.turnedIn)
        {
            continue;
        }

        if (!quest.requiredMaterialId.empty() && quest.requiredMaterialQuantity > 0)
        {
            continue;
        }

        if (quest.objectiveType != "combat" && quest.objectiveType != "bestiaire")
        {
            continue;
        }

        bool matches = false;
        for (const std::string& family : families)
        {
            if (questTargetMatchesEncounter(quest, family))
            {
                matches = true;
                break;
            }
        }

        if (!matches)
        {
            continue;
        }

        quest.progress += amount;

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


const std::vector<Quest>& QuestLog::getGuildBoardOffers() const
{
    return guildBoardOffers;
}

std::vector<Quest>& QuestLog::getGuildBoardOffers()
{
    return guildBoardOffers;
}

namespace
{
    bool questOfferMatches(const Quest& a, const Quest& b)
    {
        return a.id == b.id || a.title == b.title;
    }

    bool offerAlreadyVisible(const std::vector<Quest>& offers, const Quest& candidate)
    {
        for (const Quest& offer : offers)
        {
            if (questOfferMatches(offer, candidate))
            {
                return true;
            }
        }

        return false;
    }
}

void QuestLog::ensureGuildBoardReady(int playerLevel, int currentCombatsStarted)
{
    if (currentCombatsStarted < 0)
    {
        currentCombatsStarted = 0;
    }

    const bool expired = guildBoardCreatedAtCombat < 0
        || currentCombatsStarted - guildBoardCreatedAtCombat >= 3;

    if (expired || guildBoardOffers.empty() || guildBoardTargetSize < 3)
    {
        guildBoardOffers = QuestCatalog::createGuildBoard(playerLevel);
        guildBoardCreatedAtCombat = currentCombatsStarted;
        guildBoardTargetSize = static_cast<int>(guildBoardOffers.size());
        if (guildBoardTargetSize < 3) guildBoardTargetSize = 3;
        if (guildBoardTargetSize > 8) guildBoardTargetSize = 8;
        guildBoardPendingReplacements = 0;
        guildBoardReplacementDueAtCombat = -1;
        return;
    }

    if (guildBoardPendingReplacements > 0
        && guildBoardReplacementDueAtCombat >= 0
        && currentCombatsStarted >= guildBoardReplacementDueAtCombat)
    {
        std::vector<Quest> candidates = QuestCatalog::createGuildBoard(playerLevel);
        int added = 0;

        for (const Quest& candidate : candidates)
        {
            if (static_cast<int>(guildBoardOffers.size()) >= guildBoardTargetSize)
            {
                break;
            }

            if (hasQuest(candidate.id) || offerAlreadyVisible(guildBoardOffers, candidate))
            {
                continue;
            }

            guildBoardOffers.push_back(candidate);
            added++;
        }

        int attempts = 0;
        while (static_cast<int>(guildBoardOffers.size()) < guildBoardTargetSize && attempts < 5)
        {
            attempts++;
            std::vector<Quest> moreCandidates = QuestCatalog::createGuildBoard(playerLevel);
            for (const Quest& candidate : moreCandidates)
            {
                if (static_cast<int>(guildBoardOffers.size()) >= guildBoardTargetSize)
                {
                    break;
                }

                if (hasQuest(candidate.id) || offerAlreadyVisible(guildBoardOffers, candidate))
                {
                    continue;
                }

                guildBoardOffers.push_back(candidate);
                added++;
            }
        }

        guildBoardPendingReplacements = 0;
        guildBoardReplacementDueAtCombat = -1;
    }
}

bool QuestLog::removeGuildBoardOfferAt(int offerIndex, int currentCombatsStarted)
{
    if (offerIndex < 0 || offerIndex >= static_cast<int>(guildBoardOffers.size()))
    {
        return false;
    }

    guildBoardOffers.erase(guildBoardOffers.begin() + offerIndex);
    guildBoardPendingReplacements++;

    const int dueAt = currentCombatsStarted + 1;
    if (guildBoardReplacementDueAtCombat < 0 || dueAt < guildBoardReplacementDueAtCombat)
    {
        guildBoardReplacementDueAtCombat = dueAt;
    }

    return true;
}

int QuestLog::getGuildBoardCombatsBeforeRefresh(int currentCombatsStarted) const
{
    if (guildBoardCreatedAtCombat < 0)
    {
        return 0;
    }

    int remaining = 3 - (currentCombatsStarted - guildBoardCreatedAtCombat);
    if (remaining < 0) return 0;
    if (remaining > 3) return 3;
    return remaining;
}

int QuestLog::getGuildBoardPendingReplacements() const
{
    return guildBoardPendingReplacements;
}

int QuestLog::getGuildBoardTargetSize() const
{
    return guildBoardTargetSize;
}

int QuestLog::getGuildBoardCreatedAtCombat() const
{
    return guildBoardCreatedAtCombat;
}

int QuestLog::getGuildBoardReplacementDueAtCombat() const
{
    return guildBoardReplacementDueAtCombat;
}

int QuestLog::getClientQuestCount(const std::string& client) const
{
    int count = 0;

    for (const Quest& quest : quests)
    {
        if (!quest.guildQuest && quest.client == client)
        {
            count++;
        }
    }

    return count;
}

bool QuestLog::hasRecommendedClientCapacity(const std::string& client) const
{
    return getClientQuestCount(client) < 5;
}

void QuestLog::setLoadedGuildBoardState(
    const std::vector<Quest>& offers,
    int createdAtCombat,
    int targetSize,
    int pendingReplacements,
    int replacementDueAtCombat
)
{
    guildBoardOffers = offers;
    guildBoardCreatedAtCombat = createdAtCombat;
    guildBoardTargetSize = targetSize;
    guildBoardPendingReplacements = pendingReplacements;
    guildBoardReplacementDueAtCombat = replacementDueAtCombat;

    if (guildBoardTargetSize < static_cast<int>(guildBoardOffers.size()))
    {
        guildBoardTargetSize = static_cast<int>(guildBoardOffers.size());
    }
    if (guildBoardTargetSize < 0) guildBoardTargetSize = 0;
    if (guildBoardPendingReplacements < 0) guildBoardPendingReplacements = 0;
}

void QuestLog::clear()
{
    quests.clear();
    guildBoardOffers.clear();
    guildBoardCreatedAtCombat = -1;
    guildBoardTargetSize = 0;
    guildBoardPendingReplacements = 0;
    guildBoardReplacementDueAtCombat = -1;
}
