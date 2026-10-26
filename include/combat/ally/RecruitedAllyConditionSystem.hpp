#pragma once

#include "combat/ally/RecruitedAllyCombatRoster.hpp"

#include <string>

class Player;

struct RecruitedAllyConditionSnapshot
{
    int fatiguePercent = 0;
    int moralePercent = 50;
    int readinessPercent = 100;
    int injuryStressPercent = 0;
    int daysSinceLastStrain = 0;
    bool exhausted = false;
    bool shaken = false;
    std::string injuryStateId = "none";
    std::string injuryStateLabel = "aucune blessure persistante";
};

struct RecruitedAllyRecoveryCareResult
{
    bool applied = false;
    int fatigueBefore = 0;
    int fatigueAfter = 0;
    int injuryStressBefore = 0;
    int injuryStressAfter = 0;
    std::string injuryStateBefore;
    std::string injuryStateAfter;
};

namespace RecruitedAllyConditionSystem
{
    // Persistent fatigue is journal-backed and naturally recovers with world days.
    // It is intentionally light: it changes readiness/initiative/mood, not every tiny action.
    RecruitedAllyConditionSnapshot evaluate(const Player& player, const RecruitedAllyCombatSupport& ally);
    void applyToCombatSupport(const Player& player, RecruitedAllyCombatSupport& ally);
    void recordCombatStrain(Player& player, const RecruitedAllyCombatSupport& ally, bool bossFight);

    // Light care reduces the persistent aftermath of severe wounds/KO without magically restoring HP.
    // quality: 1 field/basic care, 2 infirmary care, 3 exceptional specialist care.
    RecruitedAllyRecoveryCareResult provideRecoveryCare(Player& player, const RecruitedAllyCombatSupport& ally, int quality = 1);
    bool needsRecoveryCare(const Player& player, const RecruitedAllyCombatSupport& ally);
    std::string summary(const RecruitedAllyConditionSnapshot& condition);
}
