#pragma once

#include "combat/ally/RecruitedAllyCombatRoster.hpp"

#include <vector>

namespace RecruitedAllyInitiativeSystem
{
    // Minimal deterministic ally initiative layer. It only orders allies inside their normal
    // action window; it never creates extra actions and never moves the enemy response.
    int baseInitiativeScore(const RecruitedAllyCombatSupport& ally);
    std::vector<int> orderedActiveIndices(const std::vector<RecruitedAllyCombatSupport>& allies);
}
