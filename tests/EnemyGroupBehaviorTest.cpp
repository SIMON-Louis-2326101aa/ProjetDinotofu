#include "combat/EnemyCombatQueue.hpp"
#include "combat/profile/MonsterBehaviorProfile.hpp"
#include "entity/Monster.hpp"
#include <cassert>

namespace
{
Monster make(const std::string& name, Race race, int level = 12)
{
    return Monster(name, "Combattant de groupe", race, level, 180, 12, 24, 36, 0, 0, false, false, false, false);
}
}

int main()
{
    const Monster leader = make("Capitaine gobelin", Race::Gobelin);
    const Monster guard = make("Gobelin porte-bouclier", Race::Gobelin);
    const Monster skirmisher = make("Gobelin pillard", Race::Gobelin);
    const Monster reserve = make("Gobelin voleur", Race::Gobelin);
    const Monster archer = make("Archer gobelin", Race::Gobelin);
    const Monster webber("Araignée tisseuse de toile", "Insecte de nid qui tend une toile", Race::Insectoide, 12, 160, 10, 22, 34, 0, 0, false, false, false, false);

    const MonsterBehaviorProfile leaderProfile = MonsterBehaviorProfileCatalog::build(leader);
    const MonsterBehaviorProfile guardProfile = MonsterBehaviorProfileCatalog::build(guard);
    const MonsterBehaviorProfile skirmisherProfile = MonsterBehaviorProfileCatalog::build(skirmisher);
    const MonsterBehaviorProfile archerProfile = MonsterBehaviorProfileCatalog::build(archer);
    const MonsterBehaviorProfile webberProfile = MonsterBehaviorProfileCatalog::build(webber);
    assert(leaderProfile.groupLeader);
    assert(leaderProfile.cooperativeGroup);
    assert(guardProfile.protectsLeader);
    assert(guardProfile.groupRole == "protecteur");
    assert(skirmisherProfile.cooperativeGroup);
    assert(skirmisherProfile.canFlee);
    assert(archerProfile.coversRetreat);
    assert(archerProfile.groupRole == "distance");
    assert(webberProfile.controlsTerrain);

    EnemyCombatQueue queue;
    queue.addWaitingEnemy(leader);
    queue.addWaitingEnemy(guard);
    queue.addWaitingEnemy(skirmisher);
    queue.addWaitingEnemy(reserve);
    queue.initializeFrontLine();
    assert(queue.getActiveEnemyCount() == EnemyCombatQueue::MAX_ACTIVE_ENEMIES);
    assert(queue.getWaitingEnemyCount() == 1);

    const std::string surrenderedName = queue.getActiveEnemy(1).getName();
    queue.removeActiveEnemyAsSurrendered(1);
    assert(queue.getSurrenderedEnemyCount() == 1);
    assert(queue.getSurrenderedEnemy(0).getName() == surrenderedName);
    assert(queue.getDefeatedEnemyCount() == 0);
    assert(queue.getEscapedEnemyCount() == 0);
    assert(queue.getActiveEnemyCount() == EnemyCombatQueue::MAX_ACTIVE_ENEMIES);
    assert(queue.getWaitingEnemyCount() == 0);

    queue.removeActiveEnemyAsEscaped(0);
    assert(queue.getEscapedEnemyCount() == 1);
    assert(queue.getSurrenderedEnemyCount() == 1);
    assert(queue.getDefeatedEnemyCount() == 0);
    return 0;
}
