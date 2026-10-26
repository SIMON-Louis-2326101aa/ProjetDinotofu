#include "combat/group/EmployerTacticalOrderSystem.hpp"
#include "core/Random.hpp"

#include <cassert>
#include <iostream>

int main()
{
    EmployerTacticalOrderContext critical;
    critical.activeEnemyCount = 3;
    critical.criticalHpAllies = 1;
    critical.lowHpAllies = 1;
    critical.reserveAvailable = true;
    critical.supportMemberActive = true;
    critical.leaderIntelligence = 95;

    bool foundExtract = false;
    for (unsigned int seed = 1; seed <= 5000 && !foundExtract; ++seed)
    {
        Random random(seed);
        const auto order = EmployerTacticalOrderSystem::decide(critical, random);
        if (order.kind == EmployerTacticalOrderKind::ExtractWounded)
        {
            foundExtract = true;
            assert(order.code == "extract_wounded");
        }
    }
    assert(foundExtract);

    EmployerTacticalOrderContext support;
    support.activeEnemyCount = 3;
    support.supportMemberActive = true;
    support.leaderIntelligence = 95;
    bool foundProtect = false;
    for (unsigned int seed = 1; seed <= 5000 && !foundProtect; ++seed)
    {
        Random random(seed);
        const auto order = EmployerTacticalOrderSystem::decide(support, random);
        if (order.kind == EmployerTacticalOrderKind::ProtectSupport)
        {
            foundProtect = true;
            assert(!order.instruction.empty());
        }
    }
    assert(foundProtect);

    EmployerTacticalOrderContext single;
    single.activeEnemyCount = 1;
    single.leaderIntelligence = 95;
    bool foundHold = false;
    for (unsigned int seed = 1; seed <= 5000 && !foundHold; ++seed)
    {
        Random random(seed);
        const auto order = EmployerTacticalOrderSystem::decide(single, random);
        if (order.kind == EmployerTacticalOrderKind::HoldLine)
        {
            foundHold = true;
            assert(EmployerTacticalOrderSystem::label(order.kind) == "Tenir la ligne");
        }
    }
    assert(foundHold);

    std::cout << "EmployerTacticalOrderSystemTest OK\n";
    return 0;
}
