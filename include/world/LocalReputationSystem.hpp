#ifndef INCLUDE_WORLD_LOCAL_REPUTATION_SYSTEM_HPP
#define INCLUDE_WORLD_LOCAL_REPUTATION_SYSTEM_HPP

#include <string>

class Player;

struct LocalReputationResult
{
    int score = 0;
    int successfulPersonalServices = 0;
    int failedPersonalServices = 0;
    int warningNotes = 0;
    std::string label = "neutre";
    int discountPercent = 0;
};

class LocalReputationSystem
{
public:
    static LocalReputationResult evaluate(const Player& player, const std::string& cityId);
    static int score(const Player& player, const std::string& cityId);
    static std::string labelForScore(int score);
    static int discountForScore(int score);
};

#endif
