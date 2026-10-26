#pragma once

#include <string>
#include <vector>

#include "world/bounty/BountyPursuitSystem.hpp"

class Player;

enum class ReaperLegendReliability
{
    Doctrine,
    Plausible,
    Folklore,
    Contradictory
};

struct ReaperLegendDefinition
{
    std::string id;
    std::string title;
    std::string text;
    ReaperLegendReliability reliability = ReaperLegendReliability::Folklore;
    int baseCredibility = 35;
};

struct ReaperHuntAssessment
{
    bool accepted = false;
    bool lethalForceJustified = false;
    bool executionExpected = false;
    bool priorityTarget = false;
    int threatScore = 0;
    std::string reason;
};

struct ReaperExecutionRecordAssessment
{
    int meritScore = 0;
    bool respectedRecord = false;
    std::string reason;
};

namespace ReaperDoctrineSystem
{
    // REAPERS-13 is a major but optional faction. This catalog intentionally separates
    // observed/declared doctrine from legends. Legends are NOT automatically canonized.
    const std::vector<ReaperLegendDefinition>& legendCatalog();
    const ReaperLegendDefinition* findLegend(const std::string& id);
    void hearLegend(Player& player, const std::string& legendId, const std::string& sourceKind = "traveler");

    // Encodes the doctrine "hunt because someone must", not "take every bounty".
    // Money/reward is deliberately absent from the decision: evidence, danger, repeats and
    // innocent risk matter; a one-off mistake can be refused/de-prioritized.
    // REAPERS-13 are shadow bounty hunters first: pursuit is prepared to close routes,
    // observe the target and prevent repeated escapes without granting supernatural certainty.
    BountyPursuitPlan planPursuit(
        int targetMobility,
        int knownEscapeAttempts,
        bool targetKnowsItIsHunted,
        bool terrainKnownToReapers,
        bool targetPreparedEscapeRoute
    );

    ReaperHuntAssessment assessHunt(
        int evidencePercent,
        int dangerPercent,
        int repeatOffenses,
        bool deliberateHarmToInnocents,
        bool activeThreat,
        bool surrenderingOrCorrectingMistake,
        bool provenIntentionalKilling = false
    );

    // Verified executions of legitimately condemned bounty targets are valued; unjustified killing is not.
    ReaperExecutionRecordAssessment assessExecutionRecord(
        int verifiedLegitimateBountyExecutions,
        int unjustifiedKills
    );
}
