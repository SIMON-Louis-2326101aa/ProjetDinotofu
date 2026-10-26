#include "combat/group/CombatLeaderRotationSystem.hpp"
#include "combat/EnemyCombatQueue.hpp"
#include "core/Random.hpp"
#include "entity/Monster.hpp"
#include "entity/Race.hpp"

#include <cassert>
#include <iostream>

int main()
{
    CombatLeaderRotationContext impossible;
    impossible.leaderPresent = false;
    impossible.activeCount = 3;
    impossible.reserveCount = 2;
    Random r0(1);
    auto d0 = CombatLeaderRotationSystem::decide(impossible, r0);
    assert(!d0.attempt);
    assert(d0.swapCount == 0);

    CombatLeaderRotationContext tactical;
    tactical.leaderPresent = true;
    tactical.activeCount = 3;
    tactical.reserveCount = 2;
    tactical.lowHpActiveCount = 2;
    tactical.criticalHpActiveCount = 1;
    tactical.leaderIntelligence = 92;
    tactical.groupSolidarity = 88;
    tactical.roundNumber = 3;

    bool foundAttempt = false;
    bool foundDouble = false;
    for (unsigned int seed = 1; seed <= 10000 && !foundDouble; ++seed)
    {
        Random random(seed);
        const auto decision = CombatLeaderRotationSystem::decide(tactical, random);
        assert(decision.chancePercent >= 6 && decision.chancePercent <= 62);
        if (decision.attempt)
        {
            foundAttempt = true;
            assert(decision.swapCount == 1 || decision.swapCount == 2);
            if (decision.swapCount == 2) foundDouble = true;
        }
    }
    assert(foundAttempt);
    assert(foundDouble);

    CombatLeaderRotationContext selfish = tactical;
    selfish.abandonsWounded = true;
    Random rs(123);
    const auto selfishDecision = CombatLeaderRotationSystem::decide(selfish, rs);
    assert(selfishDecision.chancePercent < 62);

    assert(CombatLeaderRotationSystem::refusalChancePercent(31, 10, true) == 0);
    const int lowRefusal = CombatLeaderRotationSystem::refusalChancePercent(25, 80, false);
    const int criticalRefusal = CombatLeaderRotationSystem::refusalChancePercent(5, 20, true);
    assert(lowRefusal >= 4 && lowRefusal <= 18);
    assert(criticalRefusal > lowRefusal);
    assert(criticalRefusal <= 18);

    // La file ennemie sait désormais réordonner un chef sans créer/supprimer de combattant.
    EnemyCombatQueue queue;
    queue.addWaitingEnemy(Monster("Soldat A", "Humain soldat", Race::Humain, 10, 80, 5, 9, 12, 0, 0));
    queue.addWaitingEnemy(Monster("Capitaine", "Humain capitaine chef", Race::Humain, 14, 100, 7, 12, 16, 0, 0));
    queue.addWaitingEnemy(Monster("Soldat B", "Humain soldat", Race::Humain, 9, 70, 4, 8, 11, 0, 0));
    queue.addWaitingEnemy(Monster("Réserve", "Humain soldat", Race::Humain, 8, 65, 4, 7, 10, 0, 0));
    queue.initializeFrontLine();
    assert(queue.getActiveEnemyCount() == 3);
    assert(queue.getWaitingEnemyCount() == 1);
    assert(queue.getActiveEnemy(1).getName() == "Capitaine");
    assert(queue.moveActiveEnemyToFront(1));
    assert(queue.getActiveEnemy(0).getName() == "Capitaine");
    assert(queue.getActiveEnemyCount() == 3);
    assert(queue.getWaitingEnemyCount() == 1);

    std::cout << "CombatLeaderRotationSystemTest OK\n";
    return 0;
}
