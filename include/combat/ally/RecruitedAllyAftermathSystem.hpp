#pragma once

#include "combat/ally/RecruitedAllyCombatRoster.hpp"

#include <string>
#include <vector>

class Player;
class Random;

struct RecruitedAllyAftermathDecision
{
    std::string memberName;
    int relationshipScore = 0;
    int moodScore = 50;
    int reportChancePercent = 85;
    int theftChancePercent = 0;
    bool givesDetailedReport = false;
    bool refusesDetailedReport = false;
    bool stealsPlayerShare = false;
    AllyTeamOrigin teamOrigin = AllyTeamOrigin::PersonalRecruit;
    std::string persistentMemberId;
    std::string moodLabel;
    std::string refusalReason;
};


struct RecruitedAllyLootTransportPlan
{
    int capableSurvivors = 0;
    int injuredSurvivors = 0;
    int strongCarriers = 0;
    int totalCarryUnits = 0;
    int playerTransportUnitCost = 0;
    int availableLootCarryUnits = 0;
    int portableBundleCapacity = 0;
    int recoverableEnemyBundles = 0;
    int abandonedEnemyBundles = 0;
    int heavyItemCapacity = 0;
    bool playerTransportConsumesCapacity = false;
};

struct PostKoRecoveryCandidate
{
    std::string id;
    int transportCost = 1;
    int priorityScore = 0;
    bool protectedFromAutomaticAbandon = false;
};

struct PostKoRecoverySelection
{
    std::vector<std::string> recoveredIds;
    std::vector<std::string> deferredIds;
    int usedCarryUnits = 0;
    int remainingCarryUnits = 0;
};

struct RecruitedAllyAftermathKnowledgeReport
{
    std::vector<std::string> lines;
    int directFirstRound = 0;
    int directLastRound = 0;
    int directlyObservedDefeats = 0;
    int directReserveObservationRounds = 0;
    bool observedFromReserve = false;
    bool reporterNeverEnteredCombat = false;
    bool knowsFinalOutcomeDirectly = false;
    bool usesSecondHandSource = false;
    bool hasUnknownGap = false;
    std::string secondHandSourceName;
};

namespace RecruitedAllyAftermathSystem
{
    int moodScore(const Player& player, const RecruitedAllyCombatSupport& ally);
    std::string moodLabel(int score);
    int reportChancePercent(const Player& player, const RecruitedAllyCombatSupport& ally);
    int theftChancePercent(const Player& player, const RecruitedAllyCombatSupport& ally);

    int carryingCapacityUnits(const RecruitedAllyCombatSupport& ally);

    RecruitedAllyLootTransportPlan buildLootTransportPlan(
        const std::vector<RecruitedAllyCombatSupport>& survivingAllies,
        int defeatedEnemyBundles,
        bool transportingPlayer
    );

    // Generic automatic-extraction ordering for exact objects. Protected quest/personal/unique
    // candidates are selected before ordinary loot, then by explicit recovery priority.
    PostKoRecoverySelection selectPostKoRecoveryCandidates(
        const std::vector<PostKoRecoveryCandidate>& candidates,
        int availableCarryUnits
    );

    RecruitedAllyAftermathDecision decide(
        Player& player,
        const std::vector<RecruitedAllyCombatSupport>& survivingAllies,
        Random& random,
        const std::string& contextId
    );

    // Builds only what the chosen reporter can credibly know. Earlier rounds may be filled by a
    // surviving direct witness as explicitly sourced second-hand information; otherwise they stay
    // unknown instead of being reconstructed from omniscient simulation state.
    RecruitedAllyAftermathKnowledgeReport buildKnowledgeReport(
        const RecruitedAllyAftermathDecision& decision,
        const std::vector<RecruitedAllyCombatSupport>& participants,
        int simulatedRounds,
        bool teamFinishedFight,
        int defeatedWhileUnconscious,
        int enemiesRemaining = -1
    );

    // Returns the actually delivered amount. A credible betrayal can make it zero and removes
    // the deserter from the future recruited roster through a persistent departure record.
    int deliverPlayerEconomyShare(
        Player& player,
        int owedEconomyUnits,
        const RecruitedAllyAftermathDecision& decision,
        const std::string& contextId
    );
}
