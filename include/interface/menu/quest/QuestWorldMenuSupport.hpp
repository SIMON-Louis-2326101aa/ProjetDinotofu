#pragma once

#include <string>

#include "progression/DifficultyMode.hpp"
#include "progression/DeathRuleMode.hpp"

class Player;

namespace QuestWorldMenuSupport
{
    std::string currentCityName(const Player& player);
    bool guildRequestRankDUnlocked(const Player& player);
    void openCityTravelMenu(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule);
    void openCityVault(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule);
}
