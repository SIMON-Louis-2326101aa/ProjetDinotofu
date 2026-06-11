// EN: QuestLog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: QuestLog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu.
// Description: Implements the player's quest journal.

#include "quest/QuestLog.hpp"
#include "quest/QuestCatalog.hpp"

#include <algorithm>
#include <cctype>
#include <vector>
#include <random>
#include <sstream>

namespace
{
    std::string normalizeQuestText(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        return value;
    }


    std::vector<std::string> splitLinkedQuestIds(const std::string& value)
    {
        std::vector<std::string> ids;
        std::stringstream stream(value);
        std::string id;
        while (std::getline(stream, id, '|'))
        {
            id.erase(id.begin(), std::find_if(id.begin(), id.end(), [](unsigned char c) { return !std::isspace(c); }));
            id.erase(std::find_if(id.rbegin(), id.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), id.end());
            if (!id.empty()) ids.push_back(id);
        }
        return ids;
    }

    bool linkedQuestMatchesRequiredState(const Quest& quest, const std::string& requiredState)
    {
        if (quest.failed) return false;
        if (requiredState == "known") return true;
        if (requiredState == "completed") return quest.completed || quest.turnedIn;
        return quest.turnedIn;
    }

    bool questLogTextContainsAny(const std::string& value, const std::vector<std::string>& needles)
    {
        const std::string normalized = normalizeQuestText(value);
        for (const std::string& needle : needles)
        {
            if (normalized.find(normalizeQuestText(needle)) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    std::vector<std::string> monsterQuestFamilies(const Monster& monster)
    {
        std::vector<std::string> families;

        families.push_back("Créatures locales");
        families.push_back(monster.getName());
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
            case Race::Construction:
                families.push_back("Créatures artificielles");
                families.push_back("Automate / golem / armure animée");
                families.push_back("Construction / forge");
                families.push_back("Atelier abandonné / Ruines effondrées");
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
        if (name.find("rat") != std::string::npos)
        {
            families.push_back("Rat");
            families.push_back("Rats de cave");
            families.push_back("Bêtes faibles");
        }
        if (name.find("loup") != std::string::npos || name.find("sanglier") != std::string::npos || name.find("ours") != std::string::npos)
        {
            families.push_back("Forêt ancienne / créatures naturelles");
            families.push_back("Plaine sauvage / créatures locales");
            families.push_back("Bêtes faibles");
            if (name.find("loup") != std::string::npos) families.push_back("Loup");
            if (name.find("sanglier") != std::string::npos) families.push_back("Sanglier");
            if (name.find("ours") != std::string::npos)
            {
                families.push_back("Ours");
                families.push_back("Bêtes lourdes");
            }
        }
        if (name.find("slime") != std::string::npos)
        {
            families.push_back("Slime");
            families.push_back("Slimes faibles");
            families.push_back("Marais trouble / slimes et noyés");
        }
        if (name.find("gobelin") != std::string::npos || name.find("bandit") != std::string::npos || name.find("pillard") != std::string::npos)
        {
            families.push_back("Route commerciale / humanoïdes et embuscades");
            if (name.find("gobelin") != std::string::npos) families.push_back("Gobelin");
            if (name.find("bandit") != std::string::npos || name.find("pillard") != std::string::npos) families.push_back("Bandit");
        }
        if (name.find("araignée") != std::string::npos || name.find("araignee") != std::string::npos)
        {
            families.push_back("Araignée");
            families.push_back("Insectoïdes");
            families.push_back("Insectoïdes / poison");
        }
        if (name.find("squelette") != std::string::npos || name.find("goule") != std::string::npos || name.find("revenant") != std::string::npos)
        {
            families.push_back("Ruines effondrées / morts-vivants et reliques");
            if (name.find("squelette") != std::string::npos) families.push_back("Squelette");
            if (name.find("goule") != std::string::npos) families.push_back("Goule");
            if (name.find("revenant") != std::string::npos) families.push_back("Revenant");
        }
        if (name.find("draconide") != std::string::npos || name.find("drake") != std::string::npos)
        {
            families.push_back("Draconide");
            families.push_back("Falaises des drakes gris");
        }
        if (name.find("dragon") != std::string::npos)
        {
            families.push_back("Dragon");
            families.push_back("Dragon / territoire");
        }
        if (name.find("démon") != std::string::npos || name.find("demon") != std::string::npos)
        {
            families.push_back("Démon");
            families.push_back("Menace catastrophique");
        }
        if (questLogTextContainsAny(monster.getName() + " " + monster.getType() + " " + monster.getRaceText(), {"armure", "golem", "sentinelle", "automate", "mannequin", "pantin", "statue", "totem", "épouvantail", "epouvantail", "reliure", "oiseau de pierre", "chaîne", "chaine"}))
        {
            families.push_back("Construction");
            families.push_back("Créatures artificielles");
            families.push_back("Automate / golem / armure animée");
            families.push_back("Construction / forge");
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

        const bool targetArtificial = questLogTextContainsAny(target, {"construction", "automate", "golem", "armure", "sentinelle", "mannequin", "pantin", "artificielle"});
        const bool encounterArtificial = questLogTextContainsAny(encounter, {"construction", "automate", "golem", "armure", "sentinelle", "mannequin", "pantin", "artificielle"});

        return target.find(encounter) != std::string::npos
            || encounter.find(target) != std::string::npos
            || (targetArtificial && encounterArtificial)
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
      guildBoardReplacementDueAtCombat(-1),
      guildChallengeBoardCreatedAtDay(-1)
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
        if (quest.id == questId && !quest.turnedIn && !quest.failed)
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
        if (quest.guildQuest && !quest.guildChallenge && quest.accepted && !quest.turnedIn && !quest.failed)
        {
            count++;
        }
    }

    return count;
}

int QuestLog::getActiveGuildChallengeCount() const
{
    int count = 0;
    for (const Quest& quest : quests)
    {
        if (quest.guildChallenge && quest.accepted && !quest.turnedIn && !quest.failed)
        {
            ++count;
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
        if (!quest.guildQuest && quest.client == client && quest.accepted && !quest.turnedIn && !quest.failed)
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
    return canAcceptGuildQuest(3);
}

bool QuestLog::canAcceptGuildQuest(int activeLimit) const
{
    activeLimit = std::max(1, activeLimit);
    return getActiveGuildQuestCount() < activeLimit;
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
    return addQuestWithGuildLimit(quest, 3);
}

bool QuestLog::addQuestWithGuildLimit(const Quest& quest, int activeLimit)
{
    if (hasQuest(quest.id))
    {
        return false;
    }

    if (quest.guildChallenge && getActiveGuildChallengeCount() >= 3)
    {
        return false;
    }

    if (quest.guildQuest && !quest.guildChallenge && !canAcceptGuildQuest(activeLimit))
    {
        return false;
    }

    const bool nonRefusableStoryQuest = quest.origin == "Quête principale"
        || quest.id.rfind("story_ch", 0) == 0;

    if (!quest.guildQuest && !nonRefusableStoryQuest && !canAcceptPersonalQuestForClient(quest.client))
    {
        return false;
    }

    quests.push_back(quest);
    refreshLinkedQuestProgress();
    return true;
}

// EN: progressQuest declares or implements a focused behavior used by this module.
// FR: progressQuest déclare ou implémente un comportement précis utilisé par ce module.
bool QuestLog::progressQuest(const std::string& questId, int amount)
{
    for (Quest& quest : quests)
    {
        if (quest.id == questId && quest.accepted && !quest.turnedIn && !quest.failed)
        {
            quest.progress += amount;

            if (quest.progress >= quest.target)
            {
                quest.progress = quest.target;
                quest.completed = true;
            }

            refreshLinkedQuestProgress();
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
        if (quest.id == questId && quest.accepted && !quest.turnedIn && !quest.failed)
        {
            quest.progress = quest.target;
            quest.completed = true;
            refreshLinkedQuestProgress();
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
        if (quest.id == questId && quest.completed && !quest.turnedIn && !quest.failed)
        {
            quest.turnedIn = true;
            refreshLinkedQuestProgress();
            return true;
        }
    }

    return false;
}

int QuestLog::expireOverdueQuests(int currentDay)
{
    int expired = 0;

    if (currentDay < 0)
    {
        currentDay = 0;
    }

    for (Quest& quest : quests)
    {
        if (!quest.accepted || quest.completed || quest.turnedIn || quest.failed)
        {
            continue;
        }

        if (quest.expiresAtDay < 0 || currentDay <= quest.expiresAtDay)
        {
            continue;
        }

        quest.failed = true;
        quest.failureReason = quest.guildChallenge
            ? "Défi expiré : l'exploit devait être accompli avant la fin du jour " + std::to_string(quest.expiresAtDay)
                + ". Il pourra réapparaître aléatoirement sur un futur panneau."
            : "Délai dépassé : cette demande devait être terminée avant la fin du jour "
                + std::to_string(quest.expiresAtDay) + ".";
        ++expired;
    }

    return expired;
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
        if (!quest.accepted || quest.completed || quest.turnedIn || quest.failed)
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

    refreshLinkedQuestProgress();
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
        if (!quest.accepted || quest.completed || quest.turnedIn || quest.failed)
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
        if (!quest.accepted || quest.completed || quest.turnedIn || quest.failed)
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

    refreshLinkedQuestProgress();
    return updated;
}


int QuestLog::refreshLinkedQuestProgress()
{
    int updated = 0;

    // Resolve dependency chains until they become stable. The hard upper bound keeps
    // malformed circular data deterministic while allowing aggregate quests to depend
    // on other aggregate quests without forcing a creation order.
    const int maximumPasses = std::max(1, static_cast<int>(quests.size()));
    for (int pass = 0; pass < maximumPasses; ++pass)
    {
        bool changedDuringPass = false;
        for (Quest& aggregate : quests)
        {
            if (!aggregate.retroactiveProgress || aggregate.linkedQuestIds.empty()
                || aggregate.turnedIn || aggregate.failed)
            {
                continue;
            }

            const std::vector<std::string> ids = splitLinkedQuestIds(aggregate.linkedQuestIds);
            if (ids.empty()) continue;

            int matched = 0;
            for (const std::string& id : ids)
            {
                const auto found = std::find_if(quests.begin(), quests.end(), [&](const Quest& candidate) {
                    return candidate.id == id;
                });
                if (found != quests.end() && linkedQuestMatchesRequiredState(*found, aggregate.linkedQuestRequiredState))
                {
                    ++matched;
                }
            }

            const int target = aggregate.target > 0 ? aggregate.target : static_cast<int>(ids.size());
            const int nextProgress = std::min(target, matched);
            const bool nextCompleted = nextProgress >= target;
            if (aggregate.progress != nextProgress || aggregate.completed != nextCompleted)
            {
                aggregate.progress = nextProgress;
                aggregate.completed = nextCompleted;
                ++updated;
                changedDuringPass = true;
            }
        }
        if (!changedDuringPass) break;
    }

    return updated;
}

// EN: hasTurnInReadyQuestForClient declares or implements a focused behavior used by this module.
// FR: hasTurnInReadyQuestForClient déclare ou implémente un comportement précis utilisé par ce module.
bool QuestLog::hasTurnInReadyQuestForClient(const std::string& client) const
{
    for (const Quest& quest : quests)
    {
        if (quest.client == client && quest.completed && !quest.turnedIn && !quest.failed)
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

const std::vector<Quest>& QuestLog::getGuildChallengeBoardOffers() const
{
    return guildChallengeBoardOffers;
}

std::vector<Quest>& QuestLog::getGuildChallengeBoardOffers()
{
    return guildChallengeBoardOffers;
}

void QuestLog::ensureGuildChallengeBoardReady(int playerLevel, int currentDay)
{
    if (currentDay < 0) currentDay = 0;
    if (guildChallengeBoardCreatedAtDay == currentDay)
    {
        return;
    }

    std::vector<std::string> activeConditions;
    for (const Quest& quest : quests)
    {
        if (quest.guildChallenge
            && quest.accepted
            && !quest.turnedIn
            && !quest.failed
            && !quest.challengeCondition.empty())
        {
            activeConditions.push_back(quest.challengeCondition);
        }
    }

    guildChallengeBoardOffers = QuestCatalog::createGuildChallengeBoard(playerLevel, currentDay, activeConditions);
    guildChallengeBoardCreatedAtDay = currentDay;
}

void QuestLog::forceRefreshGuildChallengeBoard(int playerLevel, int currentDay)
{
    if (currentDay < 0) currentDay = 0;

    std::vector<std::string> excludedConditions;
    for (const Quest& quest : quests)
    {
        if (quest.guildChallenge
            && quest.accepted
            && !quest.turnedIn
            && !quest.failed
            && !quest.challengeCondition.empty())
        {
            excludedConditions.push_back(quest.challengeCondition);
        }
    }
    for (const Quest& offer : guildChallengeBoardOffers)
    {
        if (!offer.challengeCondition.empty())
        {
            excludedConditions.push_back(offer.challengeCondition);
        }
    }

    guildChallengeBoardOffers = QuestCatalog::createGuildChallengeBoard(playerLevel, currentDay, excludedConditions);
    guildChallengeBoardCreatedAtDay = currentDay;
}

bool QuestLog::removeGuildChallengeBoardOfferAt(int offerIndex)
{
    if (offerIndex < 0 || offerIndex >= static_cast<int>(guildChallengeBoardOffers.size()))
    {
        return false;
    }
    guildChallengeBoardOffers.erase(guildChallengeBoardOffers.begin() + offerIndex);
    return true;
}

int QuestLog::getGuildChallengeBoardCreatedAtDay() const
{
    return guildChallengeBoardCreatedAtDay;
}

std::vector<Quest>& QuestLog::getGuildBoardOffers()
{
    return guildBoardOffers;
}

namespace
{
    int rollQuestOfferLifetimeDays()
    {
        static std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<int> distribution(3, 4);
        return distribution(generator);
    }

    void assignQuestOfferLifetime(Quest& quest, int currentDay)
    {
        if (currentDay < 0) currentDay = 0;
        quest.availableFromDay = currentDay;
        quest.expiresAtDay = currentDay + rollQuestOfferLifetimeDays();
    }

    bool isExpiredQuestOffer(const Quest& quest, int currentDay)
    {
        return quest.expiresAtDay >= 0 && currentDay >= quest.expiresAtDay;
    }

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

void QuestLog::ensureGuildBoardReady(int playerLevel, int currentDay, int targetSizeBonus)
{
    if (currentDay < 0)
    {
        currentDay = 0;
    }

    guildBoardOffers.erase(
        std::remove_if(
            guildBoardOffers.begin(),
            guildBoardOffers.end(),
            [currentDay](const Quest& offer) { return isExpiredQuestOffer(offer, currentDay); }
        ),
        guildBoardOffers.end()
    );

    const int desiredTargetSize = std::max(6, std::min(10, 6 + std::max(0, targetSizeBonus)));
    if (guildBoardTargetSize < desiredTargetSize)
    {
        guildBoardTargetSize = desiredTargetSize;
    }
    if (guildBoardTargetSize > 10)
    {
        guildBoardTargetSize = 10;
    }

    if (guildBoardCreatedAtCombat < 0)
    {
        guildBoardCreatedAtCombat = currentDay;
    }

    auto addFreshCandidates = [&](int maxAttempts) {
        int attempts = 0;
        while (static_cast<int>(guildBoardOffers.size()) < guildBoardTargetSize && attempts < maxAttempts)
        {
            attempts++;
            std::vector<Quest> candidates = QuestCatalog::createGuildBoard(playerLevel);

            for (Quest candidate : candidates)
            {
                if (static_cast<int>(guildBoardOffers.size()) >= guildBoardTargetSize)
                {
                    break;
                }

                if (hasQuest(candidate.id) || offerAlreadyVisible(guildBoardOffers, candidate))
                {
                    continue;
                }

                assignQuestOfferLifetime(candidate, currentDay);
                guildBoardOffers.push_back(candidate);
            }
        }
    };

    if (guildBoardOffers.empty())
    {
        guildBoardTargetSize = desiredTargetSize;
        guildBoardCreatedAtCombat = currentDay;
    }

    addFreshCandidates(6);

    if (guildBoardPendingReplacements > 0
        && guildBoardReplacementDueAtCombat >= 0
        && currentDay >= guildBoardReplacementDueAtCombat)
    {
        addFreshCandidates(4);
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

int QuestLog::getGuildBoardCombatsBeforeRefresh(int currentDay) const
{
    if (guildBoardOffers.empty())
    {
        return 0;
    }

    int remaining = 3;
    bool foundTimedOffer = false;

    for (const Quest& offer : guildBoardOffers)
    {
        if (offer.expiresAtDay < 0)
        {
            continue;
        }

        foundTimedOffer = true;
        remaining = std::min(remaining, offer.expiresAtDay - currentDay);
    }

    if (!foundTimedOffer) return 0;
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

    for (Quest& offer : guildBoardOffers)
    {
        if (offer.expiresAtDay < 0)
        {
            assignQuestOfferLifetime(offer, guildBoardCreatedAtCombat < 0 ? 0 : guildBoardCreatedAtCombat);
        }
    }

    if (guildBoardTargetSize < static_cast<int>(guildBoardOffers.size()))
    {
        guildBoardTargetSize = static_cast<int>(guildBoardOffers.size());
    }
    if (guildBoardTargetSize < 0) guildBoardTargetSize = 0;
    if (guildBoardTargetSize > 10) guildBoardTargetSize = 10;
    if (guildBoardPendingReplacements < 0) guildBoardPendingReplacements = 0;
}

void QuestLog::setLoadedGuildChallengeBoardState(
    const std::vector<Quest>& offers,
    int createdAtDay
)
{
    guildChallengeBoardOffers = offers;
    guildChallengeBoardCreatedAtDay = createdAtDay;
}

void QuestLog::clear()
{
    quests.clear();
    guildBoardOffers.clear();
    guildChallengeBoardOffers.clear();
    guildBoardCreatedAtCombat = -1;
    guildBoardTargetSize = 0;
    guildBoardPendingReplacements = 0;
    guildBoardReplacementDueAtCombat = -1;
    guildChallengeBoardCreatedAtDay = -1;
}
