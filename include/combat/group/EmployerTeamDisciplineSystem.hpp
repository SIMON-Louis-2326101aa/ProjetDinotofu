#pragma once

#include "combat/group/EmployerTacticalOrderSystem.hpp"

#include <string>

class Player;

struct EmployerTeamDisciplineState
{
    int professionalTrust = 70;
    int warnings = 0;
    int completedMissions = 0;
    int failedMissions = 0;
    int obeyedOrders = 0;
    int unjustifiedDisobedience = 0;
    int justifiedDisobedience = 0;
    int reserveBattlesRemaining = 0;
    std::string lastReason;
};

struct EmployerTeamOrderOutcome
{
    bool obeyed = true;
    bool justifiedDisobedience = false;
    bool preventedSeriousHarm = false;
    std::string reason;
};

enum class EmployerOrderObservationVerdict
{
    NotApplicable,
    Unobserved,
    Inconclusive,
    Obeyed,
    JustifiedDisobedience,
    UnjustifiedDisobedience
};

struct EmployerObservedOrderActionContext
{
    EmployerTacticalOrderKind orderKind = EmployerTacticalOrderKind::None;
    std::string orderCode;
    std::string actionKind;
    std::string actionTargetName;
    std::string actionTargetId;
    std::string orderedTargetName;
    std::string orderedTargetId;
    bool orderedTargetStillActive = false;
    bool targetForcedByCombatRules = false;
    bool leaderPresent = false;
    int witnessCount = 0;
    int playerHpPercent = 100;
    bool escapeSucceeded = false;
    bool immediateSurvivalThreat = false;
};


struct EmployerUnauthorizedSwitchContext
{
    std::string actorName;
    bool leaderPresent = false;
    int witnessCount = 0;
    int outgoingHpPercent = 100;
    int incomingHpPercent = 100;
    int fearPercent = 0;
    bool survivalUrgent = false;
    bool tacticalOpportunity = false;
    bool switchSucceeded = true;
};

struct EmployerObservedOrderActionAssessment
{
    EmployerOrderObservationVerdict verdict = EmployerOrderObservationVerdict::NotApplicable;
    bool directlyObservedByLeader = false;
    int credibleWitnessCount = 0;
    bool preventedSeriousHarm = false;
    std::string reason;
};

struct EmployerOrderCombatReview
{
    int observedActions = 0;
    int directLeaderObservations = 0;
    int reportedObservations = 0;
    int obeyedActions = 0;
    int justifiedDisobedienceActions = 0;
    int unjustifiedDisobedienceActions = 0;
    int inconclusiveActions = 0;
    int unobservedActions = 0;
    bool preventedSeriousHarm = false;
    std::string lastObservedReason;
    std::string lastObeyedReason;
    std::string lastJustifiedDisobedienceReason;
    std::string lastUnjustifiedDisobedienceReason;
    std::string lastInconclusiveReason;
};

struct EmployerOrderReviewOutcome
{
    EmployerOrderObservationVerdict verdict = EmployerOrderObservationVerdict::NotApplicable;
    bool disciplineRecorded = false;
    std::string reason;
    std::string reaction;
};

namespace EmployerTeamDisciplineSystem
{
    inline constexpr int InitialProfessionalTrust = 70;
    inline constexpr int ReserveTrustThreshold = 40;
    inline constexpr int DismissalTrustThreshold = 15;
    inline constexpr int MaxDisciplinarySalaryRetentionPercent = 75;

    void initializeForContract(Player& player);
    EmployerTeamDisciplineState stateFor(const Player& player);

    void recordMissionCompleted(Player& player, const std::string& missionId);
    void recordMissionFailed(Player& player, const std::string& missionId, const std::string& reason = "ordre_non_accompli");
    void recordOrderOutcome(Player& player, const EmployerTeamOrderOutcome& outcome);

    EmployerObservedOrderActionAssessment assessObservedOrderAction(
        const EmployerObservedOrderActionContext& context
    );
    EmployerObservedOrderActionAssessment assessUnauthorizedIndividualSwitch(
        const EmployerUnauthorizedSwitchContext& context
    );
    void appendObservedOrderAction(
        Player& player,
        EmployerOrderCombatReview& review,
        const EmployerObservedOrderActionContext& context,
        const std::string& combatContextId,
        int turnNumber
    );
    EmployerOrderReviewOutcome finalizeOrderReview(
        Player& player,
        const EmployerOrderCombatReview& review,
        const std::string& orderCode,
        const std::string& combatContextId
    );
    std::string orderObservationVerdictLabel(EmployerOrderObservationVerdict verdict);

    bool playerStartsInReserve(const Player& player);
    int reserveBattlesRemaining(const Player& player);
    bool consumeReserveBattle(Player& player);

    bool dismissalRecommended(const Player& player);
    int recommendedSalaryRetentionPercent(const Player& player);
    std::string standingLabel(const Player& player);
}
