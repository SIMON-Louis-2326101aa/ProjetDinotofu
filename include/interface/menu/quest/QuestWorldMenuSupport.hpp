#pragma once

#include <string>

class Player;

namespace QuestWorldMenuSupport
{
    std::string currentCityName(const Player& player);
    bool guildRequestRankDUnlocked(const Player& player);
    void openCityTravelMenu(Player& player);
    void openCityVault(Player& player);
}
