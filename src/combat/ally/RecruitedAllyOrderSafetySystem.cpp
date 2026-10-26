#include "combat/ally/RecruitedAllyOrderSafetySystem.hpp"

#include "core/Random.hpp"

#include <algorithm>

RecruitedAllyOrderSafetyDecision RecruitedAllyOrderSafetySystem::evaluate(
    const RecruitedAllyOrderSafetyContext& context,
    Random& random)
{
    RecruitedAllyOrderSafetyDecision result;
    if (!context.aggressiveForcedOrder)
    {
        result.reason = "ordre non offensif ou non forcé";
        return result;
    }

    const int hp = std::clamp(context.hpPercent, 0, 100);
    const int readiness = std::clamp(context.readinessPercent, 0, 100);
    const int morale = std::clamp(context.moralePercent, 0, 100);
    const int relationship = std::clamp(context.relationshipScore, -100, 100);
    const int enemies = std::clamp(context.enemyCount, 1, 12);

    int danger = 0;
    if (hp <= 15) danger += 48;
    else if (hp <= 30) danger += 30;
    else if (hp <= 45) danger += 14;

    if (readiness < 25) danger += 24;
    else if (readiness < 45) danger += 12;

    if (morale < 25) danger += 12;
    if (enemies >= 5) danger += 14;
    else if (enemies >= 3) danger += 7;
    if (context.bossContext) danger += 12;

    result.dangerScore = std::clamp(danger, 0, 100);
    result.catastrophic = result.dangerScore >= 82;

    // A normal risky order is still obeyed. Refusal only becomes possible when the order is
    // genuinely close to suicidal. Relationship can temper the refusal, never erase survival instinct.
    if (result.dangerScore < 65)
    {
        result.reason = "ordre risqué mais encore crédible";
        return result;
    }

    int refusal = 8 + (result.dangerScore - 65) * 2;
    refusal += std::max(0, 35 - morale) / 3;
    refusal += std::max(0, -relationship) / 5;
    refusal -= std::max(0, relationship) / 8;
    if (result.catastrophic) refusal = std::max(refusal, 30);
    result.refusalChancePercent = std::clamp(refusal, 8, 72);
    result.refused = random.between(1, 100) <= result.refusalChancePercent;
    result.reason = result.refused
        ? "la recrue juge l'ordre offensif objectivement trop dangereux et choisit de survivre"
        : "la recrue accepte malgré un risque exceptionnel";
    return result;
}
