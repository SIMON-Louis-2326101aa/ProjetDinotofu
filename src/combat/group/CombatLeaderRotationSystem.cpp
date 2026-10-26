#include "combat/group/CombatLeaderRotationSystem.hpp"

#include "core/Random.hpp"

#include <algorithm>

CombatLeaderRotationDecision CombatLeaderRotationSystem::decide(
    const CombatLeaderRotationContext& context,
    Random& random)
{
    CombatLeaderRotationDecision decision;
    if (!context.leaderPresent || context.activeCount <= 0 || context.reserveCount <= 0)
    {
        decision.reason = "aucune rotation possible";
        return decision;
    }

    int chance = 4;
    chance += std::clamp(context.leaderIntelligence, 0, 100) / 7;
    chance += std::clamp(context.groupSolidarity, 0, 100) / 9;

    if (context.lowHpActiveCount > 0) chance += 12;
    if (context.criticalHpActiveCount > 0) chance += 14;
    if (context.playerWaiting) chance += 10 + std::min(12, std::max(0, context.roundNumber - 1) * 4);
    if (context.abandonsWounded) chance -= 16;

    // Même une très bonne lecture ne doit jamais rendre la rotation systématique.
    chance = std::clamp(chance, 6, 62);
    decision.chancePercent = chance;
    if (random.between(1, 100) > chance)
    {
        decision.reason = context.abandonsWounded
            ? "le chef préfère maintenir la pression plutôt que sauver les blessés"
            : "le chef conserve sa formation actuelle";
        return decision;
    }

    decision.attempt = true;
    decision.swapCount = 1;
    if (context.reserveCount >= 2
        && context.activeCount >= 3
        && context.lowHpActiveCount >= 2
        && random.between(1, 100) <= 34)
    {
        decision.swapCount = 2;
    }
    decision.reason = context.playerWaiting
        ? "le chef ouvre une place depuis la réserve"
        : (context.criticalHpActiveCount > 0
            ? "le chef tente d'extraire un membre en danger"
            : "le chef ajuste sa composition");
    return decision;
}

int CombatLeaderRotationSystem::refusalChancePercent(int hpPercent, int loyaltyOrCohesion, bool stubborn)
{
    hpPercent = std::clamp(hpPercent, 0, 100);
    loyaltyOrCohesion = std::clamp(loyaltyOrCohesion, 0, 100);
    if (hpPercent > 30)
    {
        return 0;
    }

    int chance = 5 + (30 - hpPercent) / 3;
    chance += std::max(0, 50 - loyaltyOrCohesion) / 10;
    if (stubborn) chance += 4;
    return std::clamp(chance, 4, 18);
}

bool CombatLeaderRotationSystem::refusesRotation(
    int hpPercent,
    int loyaltyOrCohesion,
    bool stubborn,
    Random& random)
{
    const int chance = refusalChancePercent(hpPercent, loyaltyOrCohesion, stubborn);
    return chance > 0 && random.between(1, 100) <= chance;
}
