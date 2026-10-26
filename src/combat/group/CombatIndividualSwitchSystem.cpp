#include "combat/group/CombatIndividualSwitchSystem.hpp"

#include "core/Random.hpp"

#include <algorithm>

CombatIndividualSwitchDecision CombatIndividualSwitchSystem::decide(
    const CombatIndividualSwitchContext& context,
    Random& random)
{
    CombatIndividualSwitchDecision decision;
    if (!context.reserveAvailable || context.memberIsLeader)
    {
        decision.reason = context.memberIsLeader
            ? "le chef conserve sa responsabilité de ligne"
            : "aucune place de réserve n'est disponible";
        return decision;
    }

    const int hp = std::clamp(context.hpPercent, 0, 100);
    const int fear = std::clamp(context.fearPercent, 0, 100);
    const int danger = std::clamp(context.dangerPercent, 0, 100);
    const int intelligence = std::clamp(context.intelligence, 0, 100);
    const int discipline = std::clamp(context.discipline, 0, 100);
    const int loyalty = std::clamp(context.loyaltyOrCohesion, 0, 100);

    int desire = 2;
    if (hp <= 45) desire += 8;
    if (hp <= 25) desire += 12;
    if (hp <= 12) desire += 10;
    desire += fear / 12;
    desire += std::max(0, danger - 55) / 8;
    if (context.survivalUrgent) desire += 12;
    if (context.tacticalOpportunity) desire += 7 + intelligence / 20;
    if (context.stubborn) desire -= 5;
    desire += std::max(0, 45 - loyalty) / 12;
    desire = std::clamp(desire, 2, 48);
    decision.desireChancePercent = desire;

    if (random.between(1, 100) > desire)
    {
        decision.reason = "le membre reste sur sa position";
        return decision;
    }

    decision.wantsSwitch = true;

    if (!context.leaderPresent)
    {
        decision.actsWithoutPermission = true;
        decision.reason = context.survivalUrgent
            ? "sans chef disponible, l'instinct de survie prend le dessus"
            : "aucun chef présent : le membre juge seul son changement";
        return decision;
    }

    int bypass = 1;
    if (fear >= 55) bypass += (fear - 45) / 10;
    if (hp <= 20) bypass += 4;
    if (hp <= 8) bypass += 4;
    if (context.survivalUrgent) bypass += 5;
    if (context.tacticalOpportunity && intelligence >= 70) bypass += 3;
    if (context.stubborn) bypass += 2;
    bypass -= discipline / 35;
    bypass -= loyalty / 40;
    bypass = std::clamp(bypass, 1, 18);
    decision.bypassChancePercent = bypass;

    if (random.between(1, 100) <= bypass)
    {
        decision.actsWithoutPermission = true;
        if (context.survivalUrgent || fear >= 65)
            decision.reason = "peur ou instinct de survie : le membre bouge avant d'attendre l'autorisation";
        else
            decision.reason = "jugement immédiat : le membre prend la responsabilité du changement";
        return decision;
    }

    decision.asksPermission = true;
    decision.reason = "le membre demande l'autorisation du chef avant de quitter la ligne";
    return decision;
}

int CombatIndividualSwitchSystem::authorizationChancePercent(
    const CombatSwitchAuthorizationContext& context)
{
    int chance = 28;
    chance += std::clamp(context.leaderIntelligence, 0, 100) / 5;
    chance += std::clamp(context.leaderSolidarity, 0, 100) / 8;
    chance += std::clamp(context.tacticalNeedPercent, 0, 100) / 6;

    if (context.outgoingCritical) chance += 18;
    else if (context.outgoingHpPercent <= 40) chance += 9;

    if (context.incomingFresher) chance += 10;
    if (context.incomingHpPercent <= context.outgoingHpPercent) chance -= 18;

    return std::clamp(chance, 15, 92);
}

bool CombatIndividualSwitchSystem::leaderAuthorizes(
    const CombatSwitchAuthorizationContext& context,
    Random& random)
{
    return random.between(1, 100) <= authorizationChancePercent(context);
}
