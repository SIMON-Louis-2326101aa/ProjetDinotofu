// EN: Quest.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: Quest.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu.
// Description: Lightweight quest record used by the terminal quest journal.

#ifndef INCLUDE_QUEST_QUEST_HPP
#define INCLUDE_QUEST_QUEST_HPP

#include <string>
#include "economy/Money.hpp"

struct Quest
{
    std::string id;
    std::string rank;
    std::string title;
    std::string origin;
    std::string client;
    std::string location;
    std::string objective;
    std::string objectiveType;
    std::string targetFamily;
    int rewardExperience = 0;
    // Historical field name kept in saves: this is an economy-unit reward (1 unit = 1 PF), not PO.
    int rewardGold = 0;
    // Optional exact physical payout. When at least one stack is non-zero, these exact pieces
    // are awarded as authored and rewardGold is not converted or substituted.
    CoinBreakdown rewardCoins;
    std::string rewardMaterialId;
    std::string rewardMaterialName;
    int rewardMaterialQuantity = 0;
    std::string rewardNote;
    std::string requiredMaterialId;
    std::string requiredMaterialName;
    int requiredMaterialQuantity = 0;
    int progress = 0;
    int target = 0;
    bool guildQuest = false;
    bool guildChallenge = false;
    std::string challengeCondition;
    int challengeMarkReward = 0;
    int availableFromDay = 0;
    int expiresAtDay = -1;
    bool accepted = false;
    bool completed = false;
    bool turnedIn = false;
    bool failed = false;
    std::string failureReason;
    // Optional foreign-language source attached to a quest contract.
    std::string requiredLanguage;
    int requiredLanguageLevel = 0;
    std::string sourceLanguageText;

    // EN: Optional dependency metadata for multi-step or aggregate quests.
    // A pipe-separated list keeps save files simple and backward compatible.
    // FR: Métadonnées optionnelles pour les quêtes à étapes ou de synthèse.
    // Une liste séparée par | garde les sauvegardes simples et rétrocompatibles.
    std::string linkedQuestIds;
    std::string stageLabels;
    // EN: Pipe-separated ids of already completed service micro-challenges.
    // FR: Identifiants séparés par | des micro-épreuves de service déjà proposées.
    std::string serviceChallengeHistory;
    std::string linkedQuestRequiredState = "turned_in";
    bool retroactiveProgress = false;
    bool hideFutureSteps = false;

    // Optional dynamic second phase. The original objective stays visible in the journal through
    // phaseTwoPreviousSummary so a surprise follow-up never erases what the player just completed.
    bool phaseTwoChecked = false;
    bool phaseTwoOffered = false;
    bool phaseTwoActive = false;
    bool phaseTwoMandatory = false;
    bool phaseTwoCompleted = false;
    bool phaseTwoDeclined = false;
    std::string phaseTwoTitle;
    std::string phaseTwoPreviousSummary;
    std::string phaseTwoReason;
    std::string phaseTwoObjective;
    int phaseTwoProgress = 0;
    int phaseTwoTarget = 0;
    int phaseTwoAttempts = 0;
    int phaseTwoRewardExperience = 0;
    int phaseTwoRewardGold = 0;
    // Optional local follow-ups may reward reputation, relationship, information or a future opportunity
    // instead of cash. Empty/"money" keeps historical behavior.
    std::string phaseTwoRewardKind;
    std::string phaseTwoRewardDetail;
    bool phaseTwoTravelRequired = false;
    std::string phaseTwoTravelSourceCityId;
    std::string phaseTwoTravelDestinationCityId;
    int phaseTwoTravelDistanceKm = 0;

    // Optional inter-city service / transport contract. These fields are also reused by
    // ordinary quest phase 2 templates when the follow-up becomes a paid trip.
    bool intercityMission = false;
    std::string intercityKind; // information / materials
    std::string intercitySourceCityId;
    std::string intercityRouteCityIds; // ordered pipe-separated destination stops
    std::string intercityPayloadName;
    int intercityStopIndex = 0;
    int intercityTotalDistanceKm = 0;
    int intercityTravelRewardGold = 0;
    int intercityTravelRewardExperience = 0;

    // Optional live trade-incident mission (introduced by the city information office).
    bool tradeIncidentMission = false;
    std::string tradeIncidentId;
    std::string tradeIncidentKind;
    std::string tradeIncidentSourceCityId;
    std::string tradeIncidentDestinationCityId;
    int tradeIncidentGoodsType = -1;
    int tradeIncidentSeverity = 0;
    std::string tradeIncidentStage;
    int tradeIncidentAttempts = 0;
    bool tradeIncidentCreatesBountyLead = false;
    bool tradeIncidentBountyPublished = false;

    // Optional targeted-bounty runtime. These fields are ignored by ordinary quests and
    // serialized as backward-compatible optional save properties.
    bool bountyMission = false;
    // Persistent identity/social profile for authored or generated named fugitives. Old saves may
    // leave these empty; generic or group bounties remain valid without an identity profile.
    std::string bountyTargetId;
    std::string bountyTargetName;
    std::string bountyTargetTemperament;
    std::string bountyTargetHomeCityId;
    std::string bountyTargetNetworkId;
    std::string bountyIssuerId;
    std::string bountyIssuerName;
    std::string bountyIssuerOrganizationId;
    std::string bountyIssuerOrganizationName;
    bool bountyIssuerPrefersLiveCapture = true;
    std::string bountyCloseRelationId;
    std::string bountyCloseRelationName;
    std::string bountyCloseRelationType;
    std::string bountyStage; // investigation -> preparation -> pursuit -> confrontation
    int bountyClues = 0;
    int bountyInvestigationNoise = 0;
    int bountyInformantExposure = 0;
    int bountyTargetAwareness = 45;
    bool bountyTargetSuspectsHunt = false;
    bool bountyTargetKnowsHunt = false;
    int bountyTargetResources = 45;
    int bountyTargetLocalSupport = 20;
    int bountyTargetMobility = 50;
    int bountyTargetEscapeAttempts = 0;
    bool bountyTargetKnowsTerrain = false;
    bool bountyTargetInjured = false;
    int bountyExitRecon = 0;
    int bountyInformantNetwork = 0;
    int bountyAmbushPreparation = 0;
    int bountyShadowingPreparation = 0;
    int bountyRouteVariation = 0;
    int bountyDecoyQuality = 0;
    int bountyCounterSurveillance = 0;
    int bountyRefugeConcealment = 0;
    // Contract value: danger, notoriety and time at large make some targets worth much more than others.
    // Optional save fields keep old saves compatible without a schema bump.
    int bountyDangerRating = 1;
    int bountyNotorietyRating = 1;
    int bountyDaysAtLarge = 0;
    int bountyBaseRewardGold = 0;
    bool bountyRewardProfileApplied = false;

    // Regional bounty memory: optional save fields keep old saves compatible without a schema bump.
    bool bountyRegionalProfileApplied = false;
    // Persistent location for mobile targets. Kept optional so old saves load without a schema bump.
    std::string bountyCurrentCityId;
    std::string bountyCurrentBiome;
    std::string bountyVisitedCityIds; // pipe-separated, prevents silly ping-pong relocation.
    int bountyRelocationCount = 0;
    // Last inter-city trace left by a relocating target. It can reinforce one later investigation,
    // but never replaces local tracking entirely.
    std::string bountyTransitTrailKind;
    std::string bountyTransitTrailFromCityId;
    int bountyTransitTrailReliability = 0;
    bool bountyTransitTrailAvailable = false;
    std::string bountyRegionalProfileCityId; // last city whose environment/history was applied.
    bool bountyRegionalConsequencesApplied = false;
    std::string bountyResolution; // captured_surrender / captured_combat / killed_combat
};

#endif
