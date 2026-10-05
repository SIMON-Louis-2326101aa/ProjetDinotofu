#pragma once

#include <string>

class Player;

namespace QuestDeadlineSupport
{
    void expireOverdueQuestDeadlines(Player& player, const std::string& screenId, bool notify = true);
    void synchronizeQuestConsequences(Player& player);
}
