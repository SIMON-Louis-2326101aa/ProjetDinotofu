#include "combat/group/CombatIndividualSwitchSystem.hpp"
#include "core/Random.hpp"

#include <cassert>
#include <iostream>

int main()
{
    CombatIndividualSwitchContext none;
    none.reserveAvailable = false;
    Random r0(1);
    auto d0 = CombatIndividualSwitchSystem::decide(none, r0);
    assert(!d0.wantsSwitch);

    CombatIndividualSwitchContext stable;
    stable.reserveAvailable = true;
    stable.leaderPresent = true;
    stable.hpPercent = 90;
    stable.fearPercent = 0;
    stable.dangerPercent = 25;
    stable.intelligence = 60;
    stable.discipline = 80;
    stable.loyaltyOrCohesion = 80;

    bool foundRequest = false;
    for (unsigned seed = 1; seed <= 10000 && !foundRequest; ++seed)
    {
        Random random(seed);
        const auto decision = CombatIndividualSwitchSystem::decide(stable, random);
        if (decision.wantsSwitch && decision.asksPermission)
        {
            foundRequest = true;
            assert(!decision.actsWithoutPermission);
        }
    }
    assert(foundRequest);

    CombatIndividualSwitchContext panic = stable;
    panic.hpPercent = 5;
    panic.fearPercent = 95;
    panic.dangerPercent = 95;
    panic.discipline = 15;
    panic.loyaltyOrCohesion = 20;
    panic.survivalUrgent = true;
    panic.stubborn = true;

    bool foundBypass = false;
    for (unsigned seed = 1; seed <= 10000 && !foundBypass; ++seed)
    {
        Random random(seed);
        const auto decision = CombatIndividualSwitchSystem::decide(panic, random);
        if (decision.wantsSwitch && decision.actsWithoutPermission)
        {
            foundBypass = true;
            assert(decision.bypassChancePercent >= 1 && decision.bypassChancePercent <= 18);
        }
    }
    assert(foundBypass);

    CombatIndividualSwitchContext noLeader = panic;
    noLeader.leaderPresent = false;
    bool foundAutonomous = false;
    for (unsigned seed = 1; seed <= 5000 && !foundAutonomous; ++seed)
    {
        Random random(seed);
        const auto decision = CombatIndividualSwitchSystem::decide(noLeader, random);
        if (decision.wantsSwitch)
        {
            foundAutonomous = true;
            assert(decision.actsWithoutPermission);
            assert(!decision.asksPermission);
        }
    }
    assert(foundAutonomous);

    CombatSwitchAuthorizationContext weak;
    weak.leaderIntelligence = 30;
    weak.leaderSolidarity = 20;
    weak.outgoingHpPercent = 70;
    weak.incomingHpPercent = 60;
    weak.tacticalNeedPercent = 20;
    const int weakChance = CombatIndividualSwitchSystem::authorizationChancePercent(weak);

    CombatSwitchAuthorizationContext urgent = weak;
    urgent.leaderIntelligence = 90;
    urgent.leaderSolidarity = 90;
    urgent.outgoingHpPercent = 12;
    urgent.incomingHpPercent = 90;
    urgent.tacticalNeedPercent = 95;
    urgent.outgoingCritical = true;
    urgent.incomingFresher = true;
    const int urgentChance = CombatIndividualSwitchSystem::authorizationChancePercent(urgent);
    assert(weakChance >= 15 && weakChance <= 92);
    assert(urgentChance > weakChance);
    assert(urgentChance <= 92);

    std::cout << "CombatIndividualSwitchSystemTest OK\n";
    return 0;
}
