#include <cassert>

#include "world/bounty/BountyPursuitSystem.hpp"

int main()
{
    BountyPursuitHunterProfile ordinary;
    ordinary.trackingSkill = 52;
    ordinary.routeControl = 45;
    ordinary.stealthSkill = 35;
    ordinary.coordination = 50;

    BountyPursuitTargetProfile mobile;
    mobile.mobility = 80;
    mobile.knownEscapeAttempts = 0;
    mobile.knowsItIsHunted = true;
    mobile.hasPreparedEscapeRoute = true;

    const auto weak = BountyPursuitSystem::plan(ordinary, mobile);
    assert(weak.extraEscapeOpenings == 0);
    assert(weak.escapeChancePenaltyPercent < 20);

    BountyPursuitHunterProfile expert;
    expert.trackingSkill = 92;
    expert.routeControl = 90;
    expert.stealthSkill = 88;
    expert.coordination = 86;
    expert.shadowHunter = true;
    expert.capturePreferred = true;

    BountyPursuitTargetProfile repeated = mobile;
    repeated.knownEscapeAttempts = 3;
    repeated.terrainKnownToHunter = true;
    repeated.knowsItIsHunted = false;
    repeated.hasPreparedEscapeRoute = false;

    const auto strong = BountyPursuitSystem::plan(expert, repeated);
    assert(strong.hiddenApproach);
    assert(strong.containmentPercent > weak.containmentPercent);
    assert(strong.extraEscapeOpenings >= 1);
    assert(BountyPursuitSystem::adjustedEscapeChance(70, strong) < 70);
    assert(BountyPursuitSystem::adjustedRequiredOpenings(2, strong) > 2);
    assert(strong.capturePreferred);

    // V3.51.65: both sides can prepare. Hunters improve the hunt through actual work;
    // targets can reopen routes through credible counter-surveillance and decoys.
    BountyPursuitPreparation prepared;
    BountyPursuitSystem::applyHunterPreparation(prepared, BountyHunterPreparationAction::ScoutExits);
    BountyPursuitSystem::applyHunterPreparation(prepared, BountyHunterPreparationAction::UseInformant);
    BountyPursuitSystem::applyHunterPreparation(prepared, BountyHunterPreparationAction::PrepareAmbush);
    const auto preparedHunt = BountyPursuitSystem::plan(expert, repeated, prepared);
    assert(preparedHunt.hunterPreparationScore > 0);
    assert(preparedHunt.containmentPercent >= strong.containmentPercent);

    BountyPursuitSystem::applyTargetCountermeasure(prepared, BountyTargetCountermeasure::ChangeRoute);
    BountyPursuitSystem::applyTargetCountermeasure(prepared, BountyTargetCountermeasure::UseDecoy);
    BountyPursuitSystem::applyTargetCountermeasure(prepared, BountyTargetCountermeasure::CounterSurveillance);
    const auto contested = BountyPursuitSystem::plan(expert, repeated, prepared);
    assert(contested.targetCounterplayScore > 0);
    assert(contested.counterplayMeaningful);
    assert(contested.escapeChancePenaltyPercent <= preparedHunt.escapeChancePenaltyPercent);
    assert(contested.extraEscapeOpenings <= preparedHunt.extraEscapeOpenings);
    assert(BountyPursuitSystem::adjustedEscapeChance(70, contested) >= BountyPursuitSystem::adjustedEscapeChance(70, preparedHunt));

    // No amount of preparation produces a literal no-escape rule.
    assert(BountyPursuitSystem::adjustedEscapeChance(30, preparedHunt) >= 5);
    return 0;
}
