#pragma once

#include "entity/Monster.hpp"
#include "quest/Quest.hpp"

#include <string>
#include <vector>

class Player;
class Random;

enum class HestiaRescueApproach
{
    Stealth,
    Frontal
};

struct HestiaMissingPersonAvailability
{
    bool available = false;
    int completedQuestCount = 0;
    int requiredQuestCount = 20;
    std::string reason;
};

struct HestiaStealthAttempt
{
    bool success = false;
    int chancePercent = 0;
    int roll = 0;
    std::vector<std::string> lines;
};

namespace HestiaMissingPersonQuestSystem
{
    constexpr int RequiredCompletedQuests = 20;
    constexpr const char* QuestId = "story_hestia_missing_person";

    int completedQuestCount(const Player& player);
    HestiaMissingPersonAvailability availability(const Player& player);
    bool questExists(const Player& player);
    bool questActive(const Player& player);
    Quest createQuest(const Player& player);
    bool syncQuestIfAvailable(Player& player);

    bool usesSkeletonVariant(const Player& player);
    std::vector<Monster> buildDefenderWave(const Player& player);
    int stealthChancePercent(const Player& player);
    HestiaStealthAttempt attemptStealth(Player& player, Random& random);

    bool completeRescue(Player& player, const std::string& initialResidence, bool playerHasHome);
    std::vector<std::string> introLines(const Player& player);
    std::vector<std::string> approachLines(const Player& player);
}
