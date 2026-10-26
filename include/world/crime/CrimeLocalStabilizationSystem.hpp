#pragma once

#include <string>
#include <vector>

class Player;
struct Quest;

struct CrimeLocalStabilizationResult
{
    bool applied = false;
    std::string cityId;
    int strength = 0;
    int contactsCalmed = 0;
    int totalPressureReduced = 0;
    int totalRivalryReduced = 0;
    std::vector<std::string> lines;
};

namespace CrimeLocalStabilizationSystem
{
    // Applies a small, bounded local recovery when a real follow-up quest born from a
    // criminal disruption is actually completed and turned in.
    CrimeLocalStabilizationResult applyQuestTurnIn(Player& player, const Quest& quest);
}
