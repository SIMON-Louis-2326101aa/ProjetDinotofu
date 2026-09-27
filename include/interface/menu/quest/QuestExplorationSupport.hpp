#ifndef INCLUDE_INTERFACE_MENU_QUEST_QUESTEXPLORATIONSUPPORT_HPP
#define INCLUDE_INTERFACE_MENU_QUEST_QUESTEXPLORATIONSUPPORT_HPP

#include "entity/Player.hpp"
#include "core/Random.hpp"
#include "progression/DifficultyMode.hpp"
#include "progression/DeathRuleMode.hpp"
#include "quest/Quest.hpp"

#include <string>
#include <vector>

namespace QuestExplorationSupport
{
    struct MicroChallengeResult
    {
        bool success = false;
        bool partial = false;
        std::vector<std::string> lines;
    };

    std::string randomBiomeForClient(Random& random, const std::string& clientName);
    MicroChallengeResult runGuildServiceMicroChallenge(Quest& quest, Random& random);
    Quest buildNpcQuestByRoll(Player& player, int roll, std::string& intro, const std::string& biomeName = "");
    void displayQuestOffer(Player& player, const Quest& offeredQuest, const std::string& intro);
    void simulateAfterCombatMiniBoss(Player& player, Random& random, DifficultyMode difficulty, DeathRuleMode deathRule);
}

#endif
