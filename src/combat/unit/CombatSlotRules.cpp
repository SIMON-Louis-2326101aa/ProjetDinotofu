#include "combat/unit/CombatSlotRules.hpp"

#include <algorithm>

int CombatSlotRules::maxSummonsForNonSummonCount(int nonSummonCount)
{
    const int occupiedShared = std::clamp(nonSummonCount, 0, SharedActiveSlots);
    const int freeShared = SharedActiveSlots - occupiedShared;
    return freeShared + SummonOnlyBonusSlots;
}

bool CombatSlotRules::canAddUnit(
    CombatUnitKind kind,
    int currentNonSummonCount,
    int currentSummonCount
)
{
    currentNonSummonCount = std::max(0, currentNonSummonCount);
    currentSummonCount = std::max(0, currentSummonCount);

    if (kind == CombatUnitKind::Empty)
    {
        return true;
    }

    if (kind == CombatUnitKind::Summon)
    {
        return currentSummonCount < maxSummonsForNonSummonCount(currentNonSummonCount)
            && currentNonSummonCount + currentSummonCount < MaxVisibleActiveSlots;
    }

    return currentNonSummonCount < SharedActiveSlots
        && currentNonSummonCount + currentSummonCount < MaxVisibleActiveSlots;
}
