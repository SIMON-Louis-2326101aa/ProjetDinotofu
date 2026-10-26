#pragma once

#include <string>

class Player;
struct Quest;

namespace TeamEmploymentMissionSystem
{
    Quest buildCurrentMission(const Player& player);
    bool hasOpenEmployerMission(const Player& player);
    std::string openEmployerMissionId(const Player& player);
    bool assignCurrentMission(Player& player);
    int synchronizeMissionHistory(Player& player);
    bool progressServiceCombat(Player& player, int amount = 1);
    bool isEmployerMissionForCurrentContract(const Player& player, const Quest& quest);
}
