#ifndef INCLUDE_WORLD_LOCALREPUTATIONREPAIRSYSTEM_HPP
#define INCLUDE_WORLD_LOCALREPUTATIONREPAIRSYSTEM_HPP

#include <string>
#include <vector>

class Player;

class LocalReputationRepairSystem
{
public:
    static int fineCostEconomyUnits(int reputationScore);
    static int fineReputationGain(int reputationScore);
    static bool canPayFine(const Player& player, const std::string& cityId);
    static bool canPerformCommunityService(const Player& player, const std::string& cityId);
    static std::vector<std::string> payFine(Player& player, const std::string& cityId);
    static std::vector<std::string> performCommunityService(Player& player, const std::string& cityId);
};

#endif
