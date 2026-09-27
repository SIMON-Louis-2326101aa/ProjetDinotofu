#ifndef INCLUDE_INTERFACE_MENU_QUEST_QUESTMENUINTERNALSUPPORT_HPP
#define INCLUDE_INTERFACE_MENU_QUEST_QUESTMENUINTERNALSUPPORT_HPP

#include "entity/Player.hpp"
#include "entity/Monster.hpp"
#include "core/Random.hpp"
#include "progression/DifficultyMode.hpp"
#include "progression/DeathRuleMode.hpp"
#include "quest/Quest.hpp"

#include <string>
#include <utility>
#include <vector>

// Transitional shared helpers used while QuestMenu is being split into real .cpp/.hpp modules.
// They keep one implementation of legacy quest behavior instead of duplicating logic during extraction.
namespace QuestMenuInternalSupport
{
    std::vector<std::string> clientQuestAcceptedDialogueLines(const Player& player, const Quest& quest);
    std::string toLowerChoiceText(std::string text);

    bool runTrackedExplorationWave(
        Player& player,
        Random& random,
        DifficultyMode difficulty,
        DeathRuleMode deathRule,
        const std::vector<Monster>& monsters,
        const std::string& context
    );

    int askChoiceScreen(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        const std::vector<std::pair<int, std::string>>& options,
        int minChoice,
        int maxChoice,
        const std::string& invalidMessage = "Choix invalide."
    );

    void showExplorationNotice(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        bool waitAndClear = false
    );

    bool maybeTriggerLegendaryMerchantEncounter(
        Player& player,
        Random& random,
        const std::string& biomeName
    );

    void prepareQuestForAcceptance(Quest& quest, int currentDay);
    void appendDeadlineLine(std::vector<std::string>& lines, const Quest& quest, int currentDay);

    int askQuestOfferDecision(
        const std::string& title,
        const std::string& screenId,
        const Player& player,
        const Quest& quest,
        const std::vector<std::string>& introLines
    );
}

#endif
