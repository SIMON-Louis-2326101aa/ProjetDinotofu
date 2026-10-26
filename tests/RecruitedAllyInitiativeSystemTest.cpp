#include <cassert>
#include <vector>

#include "combat/ally/RecruitedAllyInitiativeSystem.hpp"

int main()
{
    RecruitedAllyCombatSupport slow;
    slow.name = "Mur";
    slow.job = "Tank lourd";
    slow.trait = "Massif et lent";
    slow.estimatedLevel = 25;
    slow.currentRankIndex = 2;

    RecruitedAllyCombatSupport fast;
    fast.name = "Flèche";
    fast.job = "Éclaireur archer";
    fast.trait = "Rapide et agile";
    fast.estimatedLevel = 20;
    fast.currentRankIndex = 1;

    assert(RecruitedAllyInitiativeSystem::baseInitiativeScore(fast)
        > RecruitedAllyInitiativeSystem::baseInitiativeScore(slow));

    std::vector<RecruitedAllyCombatSupport> allies = {slow, fast};
    auto order = RecruitedAllyInitiativeSystem::orderedActiveIndices(allies);
    assert(order.size() == 2);
    assert(order[0] == 1);
    assert(order[1] == 0);

    slow.teamLeader = true;
    allies = {fast, slow};
    order = RecruitedAllyInitiativeSystem::orderedActiveIndices(allies);
    assert(order[0] == 1); // leader priority stays stronger than speed

    return 0;
}
