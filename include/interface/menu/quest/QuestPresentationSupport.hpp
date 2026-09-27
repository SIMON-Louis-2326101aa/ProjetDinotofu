#ifndef INCLUDE_INTERFACE_MENU_QUEST_QUESTPRESENTATIONSUPPORT_HPP
#define INCLUDE_INTERFACE_MENU_QUEST_QUESTPRESENTATIONSUPPORT_HPP

#include <string>
#include <vector>
#include "quest/Quest.hpp"

namespace QuestPresentationSupport
{
    std::string questKindText(const Quest& quest);
    std::string questProgressMethodText(const Quest& quest);
    std::string lowerQuestDialogueText(std::string value);
    bool questDialogueContainsAny(const Quest& quest, const std::vector<std::string>& needles);
    bool isCountedHuntQuest(const Quest& quest);
    bool isArtificialHuntQuest(const Quest& quest);
}

#endif
