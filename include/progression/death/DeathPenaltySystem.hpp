// English: This file prepares future death penalties for non-lethal runs.
// Français : Ce fichier prépare les futures pénalités de mort pour les parties non létales.

#ifndef INCLUDE_PROGRESSION_DEATH_DEATHPENALTYSYSTEM_HPP
#define INCLUDE_PROGRESSION_DEATH_DEATHPENALTYSYSTEM_HPP

#include "core/Random.hpp"
#include "entity/Player.hpp"
#include "progression/DifficultyMode.hpp"
#include "progression/death/DeathPenaltyResult.hpp"

class DeathPenaltySystem
{
public:
    static DeathPenaltyResult applyNonLethalDeathPenalty(
        Player& player,
        DifficultyMode difficulty,
        Random& random
    );

    static void displayNonLethalDeathPenalty(
        const DeathPenaltyResult& result
    );

    static void displayLethalDeathCorruption();
    static void displayLethalCurrentDeathStatistic();
};

#endif
