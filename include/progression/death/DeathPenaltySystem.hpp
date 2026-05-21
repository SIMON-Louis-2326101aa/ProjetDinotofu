// EN: DeathPenaltySystem.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: DeathPenaltySystem.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
        Random& random,
        bool canStealEquipment = false
    );

    static void displayNonLethalDeathPenalty(
        const DeathPenaltyResult& result
    );

    // EN: displayLethalDeathCorruption declares or implements a focused behavior used by this module.
    // FR: displayLethalDeathCorruption déclare ou implémente un comportement précis utilisé par ce module.
    static void displayLethalDeathCorruption();
    // EN: displayLethalCurrentDeathStatistic declares or implements a focused behavior used by this module.
    // FR: displayLethalCurrentDeathStatistic déclare ou implémente un comportement précis utilisé par ce module.
    static void displayLethalCurrentDeathStatistic();
    // EN: displayLethalSurvivalAnomaly declares or implements a focused behavior used by this module.
    // FR: displayLethalSurvivalAnomaly déclare ou implémente un comportement précis utilisé par ce module.
    static void displayLethalSurvivalAnomaly();
};

#endif
