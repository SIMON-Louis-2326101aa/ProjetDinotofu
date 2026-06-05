// EN: StoryCampaign.hpp defines the first structured story-mode data.
// FR: StoryCampaign.hpp définit les premières données structurées du mode histoire.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_STORY_STORYCAMPAIGN_HPP
#define INCLUDE_STORY_STORYCAMPAIGN_HPP

#include "entity/Player.hpp"

#include <string>
#include <vector>

struct StoryAccessSnapshot
{
    int chapter = 0;
    int step = 0;
    int cityDevelopment = 0;
    bool sandboxModeAvailable = true;
    std::vector<std::string> unlockedSystems;
    std::vector<std::string> limitedSystems;
    std::vector<std::string> lockedSystems;
    std::vector<std::string> mainObjectives;
    std::vector<std::string> sideObjectives;
    std::vector<std::string> referentNpcs;
    std::vector<std::string> intrigueThreads;
};

class StoryCampaign
{
public:
    static StoryAccessSnapshot buildAccessSnapshot(const Player& player);
    static std::vector<std::string> buildLongIntroductionLines(const Player& player);
    static std::vector<std::string> buildWhiteFogPrologueLines(const Player& player);
    static std::vector<std::string> buildWhiteFogMemoryLossLines(const Player& player);
    static std::vector<std::string> buildWhiteFogFirstReactionLines(const Player& player, int choice);
    static std::vector<std::string> buildWhiteFogEncounterReactionLines(const Player& player, int choice);
    static std::vector<std::string> buildWhiteFogKitReactionLines(const Player& player, int choice);
    static std::vector<std::string> buildChapterOneArrivalLines(const Player& player);
    static std::vector<std::string> buildChapterOneMiraLines(const Player& player);
    static std::vector<std::string> buildChapterOneMissionLines(const Player& player);
    static std::vector<std::string> buildChapterOneProgressLines(const Player& player);
    static std::vector<std::string> buildChapterOneReferentIntroLines(const Player& player);
    static std::vector<std::string> buildChapterOneReferentTourLines(const Player& player);
    static bool canCompleteChapterOne(const Player& player);
    static std::vector<std::string> buildStoryTransitionRulesLines(const Player& player);
    static std::vector<std::string> buildStoryCompletionLines(const Player& player);
    static std::vector<std::string> buildChapterOneLines(const Player& player);
    static std::vector<std::string> buildChapterTwoLines(const Player& player);
    static std::vector<std::string> buildChapterTwoMissionLines(const Player& player);
    static std::vector<std::string> buildChapterTwoProgressLines(const Player& player);
    static std::vector<std::string> buildChapterTwoBriefingLines(const Player& player);
    static std::vector<std::string> buildChapterTwoRelaySignalLines(const Player& player);
    static std::vector<std::string> buildChapterTwoFirstRescueLines(const Player& player);
    static std::vector<std::string> buildChapterTwoRouteSackLines(const Player& player);
    static std::vector<std::string> buildChapterTwoCityRecoveryLines(const Player& player);
    static std::vector<std::string> buildChapterTwoColdInkTrailLines(const Player& player);
    static std::vector<std::string> buildChapterTwoRouteRewriteLines(const Player& player);
    static std::vector<std::string> buildChapterTwoShortRouteCounterLines(const Player& player);
    static std::vector<std::string> buildChapterTwoBlackKnotWarningLines(const Player& player);
    static std::vector<std::string> buildChapterTwoRepairDowntimeLines(const Player& player);
    static std::vector<std::string> buildChapterTwoHiddenGuardianHintLines(const Player& player);
    static std::vector<std::string> buildChapterTwoBlackKnotSealLines(const Player& player);
    static std::vector<std::string> buildChapterTwoBlackKnotScarsLines(const Player& player);
    static std::vector<std::string> buildChapterTwoGuardedRouteLines(const Player& player);
    static std::vector<std::string> buildChapterTwoSpecialThreatLines(const Player& player);
    static std::vector<std::string> buildSandboxRulesLines(const Player& player);
    static std::vector<std::string> buildDevelopmentLines(const Player& player);
    static std::vector<std::string> buildReferentNpcLines(const Player& player);
    static std::vector<std::string> buildIntrigueLines(const Player& player);
    static std::vector<std::string> buildNextObjectiveLines(const Player& player);
    static int suggestedChapterFromProgress(const Player& player);
    static int suggestedCityDevelopmentFromProgress(const Player& player);
    static bool canUnlockChapterTwo(const Player& player);
    static int maxUnlockedChapter(const Player& player);
    static bool isChapterUnlocked(const Player& player, int chapter);
};

#endif
