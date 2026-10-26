#include "combat/group/CombatReserveSpilloverSystem.hpp"
#include "combat/EnemyCombatQueue.hpp"
#include "core/Random.hpp"
#include "entity/Monster.hpp"
#include "entity/Race.hpp"

#include <cassert>
#include <iostream>

int main()
{
    assert(CombatReserveSpilloverSystem::minimumPercent(CombatReserveEffectReach::None) == 0);
    assert(CombatReserveSpilloverSystem::minimumPercent(CombatReserveEffectReach::Edge) == 5);
    assert(CombatReserveSpilloverSystem::maximumPercent(CombatReserveEffectReach::Massive) == 15);
    assert(CombatReserveSpilloverSystem::maxAffectedReserveTargets(CombatReserveEffectReach::Edge) == 1);
    assert(CombatReserveSpilloverSystem::maxAffectedReserveTargets(CombatReserveEffectReach::Wide) == 2);
    assert(CombatReserveSpilloverSystem::maxAffectedReserveTargets(CombatReserveEffectReach::Massive) == 3);

    Random random(42);
    for (int i = 0; i < 100; ++i)
    {
        const int edge = CombatReserveSpilloverSystem::rollPercent(CombatReserveEffectReach::Edge, random);
        const int wide = CombatReserveSpilloverSystem::rollPercent(CombatReserveEffectReach::Wide, random);
        const int massive = CombatReserveSpilloverSystem::rollPercent(CombatReserveEffectReach::Massive, random);
        assert(edge >= 5 && edge <= 7);
        assert(wide >= 8 && wide <= 11);
        assert(massive >= 12 && massive <= 15);
    }
    assert(CombatReserveSpilloverSystem::scaledAmount(100, 5) == 5);
    assert(CombatReserveSpilloverSystem::scaledAmount(10, 5) == 1);
    assert(CombatReserveSpilloverSystem::scaledAmount(0, 15) == 0);

    EnemyCombatQueue queue;
    queue.addWaitingEnemy(Monster("Actif A", "Humain", Race::Humain, 10, 100, 4, 8, 12, 0, 0));
    queue.addWaitingEnemy(Monster("Actif B", "Humain", Race::Humain, 10, 100, 4, 8, 12, 0, 0));
    queue.addWaitingEnemy(Monster("Actif C", "Humain", Race::Humain, 10, 100, 4, 8, 12, 0, 0));
    queue.addWaitingEnemy(Monster("Réserve", "Humain", Race::Humain, 10, 20, 4, 8, 12, 0, 0));
    queue.initializeFrontLine();
    assert(queue.getWaitingEnemyCount() == 1);
    const int dealt = queue.damageWaitingEnemy(0, 5);
    assert(dealt == 5);
    assert(queue.getWaitingEnemy(0).getHp() == 15);
    const int healed = queue.healWaitingEnemy(0, 3);
    assert(healed == 3);
    assert(queue.getWaitingEnemy(0).getHp() == 18);
    const int lethal = queue.damageWaitingEnemy(0, 99);
    assert(lethal == 18);
    assert(queue.getWaitingEnemyCount() == 0);
    assert(queue.getDefeatedEnemyCount() == 1);

    std::cout << "CombatReserveSpilloverSystemTest OK\n";
    return 0;
}
