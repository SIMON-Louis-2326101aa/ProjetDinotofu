#ifndef INCLUDE_INTERFACE_MENU_QUEST_QUESTSTORYSUPPORT_HPP
#define INCLUDE_INTERFACE_MENU_QUEST_QUESTSTORYSUPPORT_HPP

#include <string>
#include <vector>
#include "entity/Player.hpp"
#include "quest/Quest.hpp"
#include "interface/model/MenuScreen.hpp"

namespace QuestStorySupport
{
    struct StoryStepDescriptor
    {
        int number = 0;
        std::string id;
        std::string client;
        std::string title;
        std::string guidance;
    };

    void addGuidedStoryLine(MenuScreen& screen, int number, const std::string& title, const std::string& marker, const std::string& detail);
    bool isStoryReferentClientName(const std::string& clientName);
    bool hasStoryReferentReferral(const Player& player, const std::string& clientName);
    std::string storyReferentProfession(const std::string& clientName);
    std::string storyReferentRoleLine(const std::string& clientName);
    std::string storyAskHelpQuestId(const std::string& clientName);
    std::string storyMainQuestIdForClient(const std::string& clientName);
    bool isMainStoryQuest(const Quest& quest);
    bool questExistsInAnyState(const Player& player, const std::string& questId);
    bool questIsActiveInLog(const Player& player, const std::string& questId);
    bool questIsCompletedInLog(const Player& player, const std::string& questId);
    bool questIsTurnedInInLog(const Player& player, const std::string& questId);
    std::vector<std::string> splitQuestStageLabels(const std::string& value);
    std::string storyQuestStatusForId(const Player& player, const std::string& questId);
    std::string storyQuestMarkerForId(const Player& player, const std::string& questId);
    std::string storyMilestoneMarker(bool done, bool current);
    std::vector<StoryStepDescriptor> chapterTwoStoryStepDescriptors();
    std::vector<StoryStepDescriptor> chapterThreeStoryStepDescriptors();
    void addQuestGuidedStoryLine(MenuScreen& screen, const Player& player, const StoryStepDescriptor& step);
    const std::vector<std::string>& chapterOneReferentNames();
    int countKnownChapterOneReferentQuests(const Player& player);
    int countTurnedInChapterOneReferentQuests(const Player& player);
    void syncChapterOneLinkedQuestProgress(Player& player);
    std::vector<std::string> chapterOneReferentStatusLines(const Player& player);
    std::vector<std::string> questStepProgressLines(const Quest& quest);
    void prepareNonRefusableStoryQuest(Quest& quest, int currentDay);
    Quest createChapterOneMeetReferentsQuest();
    Quest createChapterOneMiraMainQuest();
    Quest createChapterOneReferentMainQuest(const std::string& clientName);
    Quest createChapterTwoBriefingQuest();
    Quest createChapterTwoNorthRoadQuest();
    Quest createChapterTwoTurnedMarkerQuest();
    Quest createChapterTwoRelayThreatQuest();
    Quest createChapterTwoRelaySignalQuest();
    Quest createChapterTwoFirstRescueQuest();
    Quest createChapterTwoRouteSackQuest();
    Quest createChapterTwoCityRecoveryQuest();
    Quest createChapterTwoColdInkTrailQuest();
    Quest createChapterTwoRouteRewriteQuest();
    Quest createChapterTwoShortRouteCounterQuest();
    Quest createChapterTwoBlackKnotWarningQuest();
    Quest createChapterTwoRepairDowntimeQuest();
    Quest createChapterTwoHiddenGuardianHintQuest();
    Quest createChapterTwoBlackKnotSealQuest();
    Quest createChapterTwoBlackKnotScarsQuest();
    Quest createChapterTwoGuardedRouteQuest();
    [[maybe_unused]] Quest createChapterThreeLonelyConvoyQuest();
    [[maybe_unused]] Quest createChapterThreeThreeRoutesQuest();
    [[maybe_unused]] Quest createChapterThreeSignaturesQuest();
    [[maybe_unused]] Quest createChapterThreeEscortWithdrawalQuest();
    [[maybe_unused]] Quest createChapterThreeMarginVillageQuest();
    [[maybe_unused]] Quest createChapterThreeCorrectedRouteQuest();
    [[maybe_unused]] Quest createChapterThreeMapGuardianQuest();
    [[maybe_unused]] Quest createChapterThreeConvoyReturnQuest();
    int countTurnedInChapterThreeRequests(const Player& player);
    int countTurnedInChapterTwoRequests(const Player& player);
    bool addNonRefusableQuestIfMissing(Player& player, Quest quest);
    bool completeAndTurnInQuestSilently(Player& player, const std::string& questId);
    int countTurnedInChapterOneMainRequests(const Player& player);
    bool handleStoryReferentMainQuestDialogue(Player& player, const std::string& clientName);
}

#endif
