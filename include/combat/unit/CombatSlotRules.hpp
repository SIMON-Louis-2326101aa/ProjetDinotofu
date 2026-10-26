// EN: Shared combat-slot rules used by players, allied AI and summons.
// FR: Règles communes de places de combat pour joueurs, IA alliées et invocations.
#pragma once

#include "combat/group/CombatUnitKind.hpp"

class CombatSlotRules
{
public:
    // Three shared places can be occupied by players, allied AI or summons.
    // Players/regular fighters keep priority when groups are built.
    inline static constexpr int SharedActiveSlots = 3;

    // Two extra places exist only for summons.
    inline static constexpr int SummonOnlyBonusSlots = 2;
    inline static constexpr int MaxVisibleActiveSlots = SharedActiveSlots + SummonOnlyBonusSlots;

    static int maxSummonsForNonSummonCount(int nonSummonCount);
    static bool canAddUnit(
        CombatUnitKind kind,
        int currentNonSummonCount,
        int currentSummonCount
    );
};
