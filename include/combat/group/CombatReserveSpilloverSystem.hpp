#pragma once

#include <string>

class Random;

enum class CombatReserveEffectReach
{
    None,
    Edge,
    Wide,
    Massive
};

struct CombatReserveSpillover
{
    int percent = 0;
    int amount = 0;
};

namespace CombatReserveSpilloverSystem
{
    int minimumPercent(CombatReserveEffectReach reach);
    int maximumPercent(CombatReserveEffectReach reach);
    int rollPercent(CombatReserveEffectReach reach, Random& random);
    int scaledAmount(int sourceAmount, int percent);
    int maxAffectedReserveTargets(CombatReserveEffectReach reach);
    CombatReserveSpillover roll(int sourceAmount, CombatReserveEffectReach reach, Random& random);
    std::string reachLabel(CombatReserveEffectReach reach);
}
