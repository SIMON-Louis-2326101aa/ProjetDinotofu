#include "combat/group/CombatReserveSpilloverSystem.hpp"

#include "core/Random.hpp"

#include <algorithm>

int CombatReserveSpilloverSystem::minimumPercent(CombatReserveEffectReach reach)
{
    switch (reach)
    {
        case CombatReserveEffectReach::Edge: return 5;
        case CombatReserveEffectReach::Wide: return 8;
        case CombatReserveEffectReach::Massive: return 12;
        case CombatReserveEffectReach::None:
        default: return 0;
    }
}

int CombatReserveSpilloverSystem::maximumPercent(CombatReserveEffectReach reach)
{
    switch (reach)
    {
        case CombatReserveEffectReach::Edge: return 7;
        case CombatReserveEffectReach::Wide: return 11;
        case CombatReserveEffectReach::Massive: return 15;
        case CombatReserveEffectReach::None:
        default: return 0;
    }
}

int CombatReserveSpilloverSystem::rollPercent(CombatReserveEffectReach reach, Random& random)
{
    const int low = minimumPercent(reach);
    const int high = maximumPercent(reach);
    if (high <= 0) return 0;
    return random.between(low, high);
}

int CombatReserveSpilloverSystem::scaledAmount(int sourceAmount, int percent)
{
    if (sourceAmount <= 0 || percent <= 0) return 0;
    return std::max(1, sourceAmount * std::clamp(percent, 0, 100) / 100);
}

int CombatReserveSpilloverSystem::maxAffectedReserveTargets(CombatReserveEffectReach reach)
{
    switch (reach)
    {
        case CombatReserveEffectReach::Edge: return 1;
        case CombatReserveEffectReach::Wide: return 2;
        case CombatReserveEffectReach::Massive: return 3;
        case CombatReserveEffectReach::None:
        default: return 0;
    }
}

CombatReserveSpillover CombatReserveSpilloverSystem::roll(
    int sourceAmount,
    CombatReserveEffectReach reach,
    Random& random)
{
    CombatReserveSpillover result;
    result.percent = rollPercent(reach, random);
    result.amount = scaledAmount(sourceAmount, result.percent);
    return result;
}

std::string CombatReserveSpilloverSystem::reachLabel(CombatReserveEffectReach reach)
{
    switch (reach)
    {
        case CombatReserveEffectReach::Edge: return "bord de zone";
        case CombatReserveEffectReach::Wide: return "zone large";
        case CombatReserveEffectReach::Massive: return "zone massive";
        case CombatReserveEffectReach::None:
        default: return "hors de portée";
    }
}
