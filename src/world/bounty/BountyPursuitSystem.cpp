#include "world/bounty/BountyPursuitSystem.hpp"

#include <algorithm>

namespace
{
    int cappedPrep(int value)
    {
        return std::clamp(value, 0, 2);
    }
}

void BountyPursuitSystem::applyHunterPreparation(BountyPursuitPreparation& preparation, BountyHunterPreparationAction action)
{
    switch (action)
    {
        case BountyHunterPreparationAction::ScoutExits:
            preparation.exitRecon = cappedPrep(preparation.exitRecon + 1);
            break;
        case BountyHunterPreparationAction::UseInformant:
            preparation.informantNetwork = cappedPrep(preparation.informantNetwork + 1);
            break;
        case BountyHunterPreparationAction::PrepareAmbush:
            preparation.ambushPreparation = cappedPrep(preparation.ambushPreparation + 1);
            break;
        case BountyHunterPreparationAction::ShadowTarget:
            preparation.shadowingPreparation = cappedPrep(preparation.shadowingPreparation + 1);
            break;
    }
}

void BountyPursuitSystem::applyTargetCountermeasure(BountyPursuitPreparation& preparation, BountyTargetCountermeasure action)
{
    switch (action)
    {
        case BountyTargetCountermeasure::ChangeRoute:
            preparation.routeVariation = cappedPrep(preparation.routeVariation + 1);
            break;
        case BountyTargetCountermeasure::UseDecoy:
            preparation.decoyQuality = cappedPrep(preparation.decoyQuality + 1);
            break;
        case BountyTargetCountermeasure::CounterSurveillance:
            preparation.counterSurveillance = cappedPrep(preparation.counterSurveillance + 1);
            break;
        case BountyTargetCountermeasure::HideOrChangeRefuge:
            preparation.refugeConcealment = cappedPrep(preparation.refugeConcealment + 1);
            break;
    }
}

BountyPursuitPlan BountyPursuitSystem::plan(
    const BountyPursuitHunterProfile& hunter,
    const BountyPursuitTargetProfile& target,
    const BountyPursuitPreparation& preparation)
{
    BountyPursuitPlan result;
    const int tracking = std::clamp(hunter.trackingSkill, 0, 100);
    const int routes = std::clamp(hunter.routeControl, 0, 100);
    const int stealth = std::clamp(hunter.stealthSkill, 0, 100);
    const int coordination = std::clamp(hunter.coordination, 0, 100);
    const int mobility = std::clamp(target.mobility, 0, 100);
    const int escapes = std::clamp(target.knownEscapeAttempts, 0, 12);

    const int exitRecon = cappedPrep(preparation.exitRecon);
    const int informants = cappedPrep(preparation.informantNetwork);
    const int ambush = cappedPrep(preparation.ambushPreparation);
    const int shadowing = cappedPrep(preparation.shadowingPreparation);
    const int routeVariation = cappedPrep(preparation.routeVariation);
    const int decoys = cappedPrep(preparation.decoyQuality);
    const int counterSurveillance = cappedPrep(preparation.counterSurveillance);
    const int refugeConcealment = cappedPrep(preparation.refugeConcealment);

    result.hunterPreparationScore = std::clamp(
        exitRecon * 13 + informants * 14 + ambush * 14 + shadowing * 10,
        0, 100);
    result.targetCounterplayScore = std::clamp(
        routeVariation * 14 + decoys * 15 + counterSurveillance * 13 + refugeConcealment * 11,
        0, 100);
    result.counterplayMeaningful = result.targetCounterplayScore >= 15;

    result.trackingScore = std::clamp(
        tracking
        + (target.terrainKnownToHunter ? 12 : 0)
        + escapes * 4
        + informants * 7
        + shadowing * 5
        + exitRecon * 3
        - (target.knowsItIsHunted ? 8 : 0)
        - decoys * 7
        - counterSurveillance * 6
        - refugeConcealment * 5,
        0, 100);

    result.containmentPercent = std::clamp(
        routes * 45 / 100
        + coordination * 30 / 100
        + result.trackingScore * 25 / 100
        + (hunter.shadowHunter ? 8 : 0)
        + escapes * 3
        + exitRecon * 7
        + ambush * 8
        + informants * 3
        - mobility * 22 / 100
        - (target.hasPreparedEscapeRoute ? 12 : 0)
        - routeVariation * 7
        - decoys * 4
        - refugeConcealment * 3,
        0, 100);

    result.hiddenApproach = hunter.shadowHunter
        && stealth + shadowing * 7 >= 65
        && (!target.knowsItIsHunted || stealth + shadowing * 8 + 10 >= mobility + counterSurveillance * 9);
    if (result.hiddenApproach)
    {
        result.containmentPercent = std::clamp(result.containmentPercent + 8, 0, 100);
    }

    // Counterplay can never make preparation irrelevant, but it can meaningfully reopen routes.
    // Likewise, preparation can never create a literal 0% escape state.
    result.escapeChancePenaltyPercent = std::clamp(
        result.containmentPercent / 3 - result.targetCounterplayScore / 10,
        0, 30);
    const int rawExtraOpenings = result.containmentPercent >= 82 ? 2
        : result.containmentPercent >= 58 ? 1
        : 0;
    result.extraEscapeOpenings = std::clamp(rawExtraOpenings - (result.targetCounterplayScore >= 58 ? 1 : 0), 0, 2);
    result.hardContainment = result.containmentPercent >= 78 && result.targetCounterplayScore < 70;
    result.capturePreferred = hunter.capturePreferred;

    if (result.containmentPercent >= 82 && result.targetCounterplayScore < 45)
        result.summary = "Les issues principales sont déjà couvertes : fuir exige plusieurs ouvertures successives.";
    else if (result.containmentPercent >= 58 && result.targetCounterplayScore >= 35)
        result.summary = "La traque ferme plusieurs routes, mais les leurres et changements d'itinéraire ont rouvert de vraies options de fuite.";
    else if (result.containmentPercent >= 58)
        result.summary = "La traque ferme progressivement les routes évidentes et réduit les options de fuite.";
    else if (result.trackingScore >= 60)
        result.summary = "La cible reste suivie, mais plusieurs sorties crédibles demeurent ouvertes.";
    else if (result.targetCounterplayScore >= 45)
        result.summary = "Le contre-pistage brouille sérieusement la chasse : le chasseur doit reconstruire une piste crédible.";
    else
        result.summary = "La piste reste fragile : le chasseur doit encore observer avant de prétendre verrouiller la zone.";

    return result;
}

int BountyPursuitSystem::adjustedEscapeChance(int baseChancePercent, const BountyPursuitPlan& plan)
{
    return std::clamp(baseChancePercent - std::clamp(plan.escapeChancePenaltyPercent, 0, 30), 5, 95);
}

int BountyPursuitSystem::adjustedRequiredOpenings(int baseRequiredOpenings, const BountyPursuitPlan& plan)
{
    return std::clamp(std::max(1, baseRequiredOpenings) + std::clamp(plan.extraEscapeOpenings, 0, 2), 1, 6);
}
