#include "world/bounty/BountyTargetBehaviorSystem.hpp"

#include <cassert>
#include <iostream>

int main()
{
    BountyTargetBehaviorContext unaware;
    unaware.awareness = 10;
    unaware.preparationWindow = 3;
    const auto none = BountyTargetBehaviorSystem::chooseCountermeasures(unaware);
    assert(none.selectedActions.empty());


    // Even a very alert target does not gain anti-hunt bonuses if nobody has given it a reason
    // to believe a hunt is actually underway.
    BountyTargetBehaviorContext alertButUnaware;
    alertButUnaware.temperament = "méfiant";
    alertButUnaware.resources = 90;
    alertButUnaware.awareness = 95;
    alertButUnaware.preparationWindow = 4;
    const auto routineOnly = BountyTargetBehaviorSystem::chooseCountermeasures(alertButUnaware);
    assert(routineOnly.selectedActions.empty());

    BountyTargetBehaviorContext suspicious = alertButUnaware;
    suspicious.suspectsSurveillance = true;
    const auto cautiousCheck = BountyTargetBehaviorSystem::chooseCountermeasures(suspicious);
    assert(cautiousCheck.selectedActions.size() <= 1);
    for (const auto action : cautiousCheck.selectedActions)
    {
        assert(action == BountyTargetCountermeasure::CounterSurveillance
            || action == BountyTargetCountermeasure::ChangeRoute);
    }

    BountyTargetBehaviorContext cautious;
    cautious.temperament = "survivant prudent et méthodique";
    cautious.resources = 75;
    cautious.awareness = 90;
    cautious.knowsItIsHunted = true;
    cautious.preparationWindow = 3;
    cautious.localSupport = 40;
    cautious.mobility = 80;
    cautious.previousEscapeAttempts = 2;
    cautious.knowsTerrain = true;
    const auto prepared = BountyTargetBehaviorSystem::chooseCountermeasures(cautious);
    assert(prepared.selectedActions.size() >= 2);
    assert(prepared.preparation.routeVariation > 0 || prepared.preparation.counterSurveillance > 0);
    assert(prepared.counterplayScore > 0);

    BountyTargetBehaviorContext poor;
    poor.temperament = "rusé";
    poor.resources = 10;
    poor.awareness = 80;
    poor.knowsItIsHunted = true;
    poor.preparationWindow = 4;
    const auto limited = BountyTargetBehaviorSystem::chooseCountermeasures(poor);
    assert(limited.selectedActions.size() <= 1);

    BountyPursuitHunterProfile hunter;
    hunter.trackingSkill = 88;
    hunter.routeControl = 86;
    hunter.stealthSkill = 82;
    hunter.coordination = 84;
    BountyPursuitTargetProfile target;
    target.mobility = cautious.mobility;
    target.knowsItIsHunted = true;
    target.hasPreparedEscapeRoute = true;
    const auto noCounter = BountyPursuitSystem::plan(hunter, target);
    const auto withCounter = BountyPursuitSystem::plan(hunter, target, prepared.preparation);
    assert(withCounter.targetCounterplayScore > noCounter.targetCounterplayScore);
    assert(BountyPursuitSystem::adjustedEscapeChance(70, withCounter) >= BountyPursuitSystem::adjustedEscapeChance(70, noCounter));

    std::cout << "BountyTargetBehaviorSystemTest OK\n";
    return 0;
}
