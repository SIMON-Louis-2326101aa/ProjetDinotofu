#include <cassert>

#include "combat/ally/RecruitedAllyOrderSafetySystem.hpp"
#include "core/Random.hpp"

int main()
{
    RecruitedAllyOrderSafetyContext safe;
    safe.hpPercent = 85;
    safe.readinessPercent = 90;
    safe.moralePercent = 70;
    safe.enemyCount = 2;
    safe.aggressiveForcedOrder = true;
    Random safeRandom(7);
    const auto safeDecision = RecruitedAllyOrderSafetySystem::evaluate(safe, safeRandom);
    assert(safeDecision.dangerScore < 65);
    assert(safeDecision.refusalChancePercent == 0);
    assert(!safeDecision.refused);

    RecruitedAllyOrderSafetyContext suicidal;
    suicidal.hpPercent = 8;
    suicidal.readinessPercent = 15;
    suicidal.moralePercent = 15;
    suicidal.enemyCount = 6;
    suicidal.bossContext = true;
    suicidal.aggressiveForcedOrder = true;
    suicidal.relationshipScore = 0;

    bool refusalObserved = false;
    for (unsigned int seed = 1; seed < 100 && !refusalObserved; ++seed)
    {
        Random random(seed);
        const auto decision = RecruitedAllyOrderSafetySystem::evaluate(suicidal, random);
        assert(decision.catastrophic);
        assert(decision.refusalChancePercent >= 30);
        refusalObserved = decision.refused;
    }
    assert(refusalObserved);

    RecruitedAllyOrderSafetyContext defensive = suicidal;
    defensive.aggressiveForcedOrder = false;
    Random defensiveRandom(1);
    const auto defensiveDecision = RecruitedAllyOrderSafetySystem::evaluate(defensive, defensiveRandom);
    assert(!defensiveDecision.refused);
    assert(defensiveDecision.refusalChancePercent == 0);
    return 0;
}
