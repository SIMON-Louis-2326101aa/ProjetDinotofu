#pragma once

#include <vector>

class Player;
class Random;

struct LethalSpiritResistanceResult
{
    bool available = false;
    bool consumed = false;
    bool survived = false;
    int successes = 0;
    int failures = 0;
    std::vector<int> rolls;
};

namespace LethalSpiritResistanceSystem
{
    constexpr int RequiredSuccesses = 3;
    constexpr int RequiredFailures = 3;
    constexpr int D20SuccessThreshold = 13;

    bool hasAlreadyConsumedFirstResistance(const Player& player);
    bool canAttempt(const Player& player);

    LethalSpiritResistanceResult tryFirstLethalResistance(
        Player& player,
        Random& random,
        const char* contextId
    );
}
