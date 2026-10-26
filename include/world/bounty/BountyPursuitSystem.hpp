#pragma once

#include <string>

enum class BountyHunterPreparationAction
{
    ScoutExits,
    UseInformant,
    PrepareAmbush,
    ShadowTarget
};

enum class BountyTargetCountermeasure
{
    ChangeRoute,
    UseDecoy,
    CounterSurveillance,
    HideOrChangeRefuge
};

struct BountyPursuitPreparation
{
    int exitRecon = 0;
    int informantNetwork = 0;
    int ambushPreparation = 0;
    int shadowingPreparation = 0;

    int routeVariation = 0;
    int decoyQuality = 0;
    int counterSurveillance = 0;
    int refugeConcealment = 0;
};

struct BountyPursuitHunterProfile
{
    int trackingSkill = 50;
    int routeControl = 50;
    int stealthSkill = 50;
    int coordination = 50;
    bool shadowHunter = false;
    bool capturePreferred = true;
};

struct BountyPursuitTargetProfile
{
    int mobility = 50;
    int knownEscapeAttempts = 0;
    bool knowsItIsHunted = false;
    bool terrainKnownToHunter = false;
    bool hasPreparedEscapeRoute = false;
};

struct BountyPursuitPlan
{
    int trackingScore = 0;
    int containmentPercent = 0;
    int escapeChancePenaltyPercent = 0;
    int extraEscapeOpenings = 0;
    int hunterPreparationScore = 0;
    int targetCounterplayScore = 0;
    bool hiddenApproach = false;
    bool hardContainment = false;
    bool capturePreferred = true;
    bool counterplayMeaningful = false;
    std::string summary;
};

namespace BountyPursuitSystem
{
    // Generic bounty-hunter pursuit logic. No faction gets a magical no-escape switch:
    // strong hunters reduce escape odds by preparation, route control and repeated observation.
    BountyPursuitPlan plan(
        const BountyPursuitHunterProfile& hunter,
        const BountyPursuitTargetProfile& target,
        const BountyPursuitPreparation& preparation = {}
    );

    // Preparation is deliberately capped and reversible by target counterplay. These helpers are
    // intended for future bounty quests, informants, ambushes and escape-route choices.
    void applyHunterPreparation(BountyPursuitPreparation& preparation, BountyHunterPreparationAction action);
    void applyTargetCountermeasure(BountyPursuitPreparation& preparation, BountyTargetCountermeasure action);

    int adjustedEscapeChance(int baseChancePercent, const BountyPursuitPlan& plan);
    int adjustedRequiredOpenings(int baseRequiredOpenings, const BountyPursuitPlan& plan);
}
