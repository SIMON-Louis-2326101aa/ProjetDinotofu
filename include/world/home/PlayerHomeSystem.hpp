#pragma once

#include <string>
#include <vector>

class Player;

struct PlayerHomePurchasePlan
{
    bool valid = false;
    bool fullyPurchased = false;
    int upfrontCost = 0;
    int recurringRent = 0;
    std::string reason;
};

namespace PlayerHomeSystem
{
    PlayerHomePurchasePlan planPurchase(const Player& player, const std::string& cityId, bool buyOutright);
    bool registerPurchase(Player& player, const std::string& cityId, bool buyOutright, int currentDay);
    bool isRentDue(const Player& player, int currentDay);
    bool servicesActive(const Player& player, int currentDay);
    bool payRent(Player& player, int currentDay);
    bool canHouseHestia(const Player& player);
    std::vector<std::string> describeResidents(const Player& player);

    // Relationship foundations only: the dedicated ring quest and full romance flow remain separate.
    bool registerNpcSpouse(Player& player, const std::string& npcName, bool specialRingQuestCompleted);
    bool registerPlayerSpouse(Player& player, const std::string& playerId, bool specialRingQuestCompleted);
}
