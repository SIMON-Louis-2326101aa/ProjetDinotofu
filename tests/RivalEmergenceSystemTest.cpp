#include "combat/rival/RivalEmergenceSystem.hpp"

#include <cassert>
#include <iostream>

int main()
{
    RivalEmergenceContext ordinary;
    ordinary.behaviorArchetype = "Gobelin opportuniste";
    assert(RivalEmergenceSystem::calculateChancePercent(ordinary) == 6);
    assert(!RivalEmergenceSystem::evaluate(ordinary, 7).becomesRival);
    assert(RivalEmergenceSystem::evaluate(ordinary, 6).becomesRival);

    RivalEmergenceContext marked = ordinary;
    marked.elite = true;
    marked.evolved = true;
    marked.witnessed = true;
    marked.memoryKept = true;
    marked.rivalOath = true;
    marked.priorTraces = 10;
    marked.remainingHpPercent = 1;
    assert(RivalEmergenceSystem::calculateChancePercent(marked) == 65);
    assert(!RivalEmergenceSystem::evaluate(marked, 66).becomesRival);

    RivalEmergenceContext ineligible = marked;
    ineligible.eligible = false;
    assert(RivalEmergenceSystem::calculateChancePercent(ineligible) == 0);
    assert(!RivalEmergenceSystem::evaluate(ineligible, 1).becomesRival);

    assert(RivalEmergenceSystem::minimumReturnDelayDays(0) == 2);
    assert(RivalEmergenceSystem::minimumReturnDelayDays(8) == 6);
    assert(RivalEmergenceSystem::returnChancePercent(true, false, false, 0, 1) == 0);
    assert(RivalEmergenceSystem::returnChancePercent(true, false, false, 0, 2) == 18);
    assert(RivalEmergenceSystem::returnChancePercent(false, true, true, 0, 2) == 15);

    std::cout << "RivalEmergenceSystem: OK\n";
    return 0;
}
