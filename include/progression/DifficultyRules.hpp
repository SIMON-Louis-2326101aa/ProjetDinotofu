// English: This file prepares difficulty-dependent balancing rules.
// Français : Ce fichier prépare les règles d'équilibrage dépendantes de la difficulté.

#ifndef INCLUDE_PROGRESSION_DIFFICULTYRULES_HPP
#define INCLUDE_PROGRESSION_DIFFICULTYRULES_HPP

#include "progression/DifficultyMode.hpp"

class DifficultyRules
{
public:
    static int getPlayerPveEscapeDefeatedRewardPercentage(DifficultyMode difficulty);
    static int getPlayerPveEscapeDamagedAliveRewardPercentage(DifficultyMode difficulty);
    static int getNonLethalDeathInventoryLossPercentage(DifficultyMode difficulty);
    static int getNonLethalDeathGoldLossPercentage(DifficultyMode difficulty);
    static int getNonLethalDeathExperienceLossPercentage(DifficultyMode difficulty);

    static bool isPermanentDeath(DifficultyMode difficulty);
};

#endif
