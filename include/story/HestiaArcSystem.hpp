#pragma once

#include <string>
#include <vector>

class Player;
class Random;

struct HestiaRescueScenario
{
    std::string variantId;
    std::string defenders;
    std::string eliteThreat;
    bool stealthPossible = true;
    int stealthDifficulty = 78;
    std::vector<std::string> warnings;
};

struct HestiaSmallFind
{
    bool occurred = false;
    std::string label;
    std::string materialId;
    int materialQuantity = 0;
    int copper = 0;
    std::vector<std::string> lines;
};

namespace HestiaArcSystem
{
    HestiaRescueScenario rescueScenario(bool skeletonVariant);
    bool completeMainStoryRescue(Player& player, const std::string& initialResidence, bool playerHasHome);
    bool moveResidence(Player& player, const std::string& residence, bool playerHasHome);
    std::vector<std::string> visit(Player& player, int currentDay, Random& random);
    HestiaSmallFind maybeBringSmallFind(Player& player, int currentDay, Random& random);
    std::string wellbeingLabel(const Player& player, int currentDay);

    bool setAwakeningStage(Player& player, int stage);
    int combatAptitudeScore(const Player& player, const std::string& discipline);
    bool canSacrificeForPlayer(const Player& player, bool hestiaActiveInParty);
    bool consumeSacrificeForPlayer(Player& player, bool hestiaActiveInParty);
}
