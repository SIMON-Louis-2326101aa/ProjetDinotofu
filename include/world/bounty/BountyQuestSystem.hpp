#pragma once

#include "quest/Quest.hpp"
#include "world/bounty/BountyHuntAwarenessSystem.hpp"
#include "world/bounty/BountyPursuitSystem.hpp"

#include <string>
#include <vector>

enum class BountyInvestigationAction
{
    ObserveQuietly,
    AskAround,
    CheckExits,
    PrepareInterception
};

struct BountyQuestStepResult
{
    bool changed = false;
    bool pursuitAvailable = false;
    bool pursuitAttempted = false;
    bool intercepted = false;
    bool escaped = false;
    bool transitTrailEvaluated = false;
    bool transitTrailUseful = false;
    int cluesGained = 0;
    int escapeChancePercent = 0;
    BountyTargetAwarenessState awarenessState = BountyTargetAwarenessState::Unaware;
    BountyPursuitPlan pursuitPlan;
    std::vector<std::string> lines;
};

namespace BountyQuestSystem
{
    bool isActiveBounty(const Quest& quest);
    void initializeBounty(Quest& quest, int playerLevel);

    BountyPursuitPreparation readPreparation(const Quest& quest);
    void writePreparation(Quest& quest, const BountyPursuitPreparation& preparation);

    BountyQuestStepResult investigate(
        Quest& quest,
        BountyInvestigationAction action,
        int hunterStealth,
        int roll1To100
    );

    BountyQuestStepResult attemptPursuit(
        Quest& quest,
        const BountyPursuitHunterProfile& hunter,
        int escapeRoll1To100
    );

    std::string stageLabel(const Quest& quest);
    std::string awarenessLabel(const Quest& quest);
    std::string investigationActionLabel(BountyInvestigationAction action);
    std::string dangerLabel(const Quest& quest);
    std::string notorietyLabel(const Quest& quest);
    std::string contractValueSummary(const Quest& quest);
}
