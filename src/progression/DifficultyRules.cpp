// English: This file prepares difficulty-dependent balancing rules.
// Français : Ce fichier prépare les règles d'équilibrage dépendantes de la difficulté.

#include "progression/DifficultyRules.hpp"

int DifficultyRules::getPlayerPveEscapeDefeatedRewardPercentage(DifficultyMode difficulty)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return 90;

        case DifficultyMode::Hard:
            return 35;

        case DifficultyMode::Lethal:
            return 25;

        case DifficultyMode::Normal:
        default:
            return 50;
    }
}

int DifficultyRules::getPlayerPveEscapeDamagedAliveRewardPercentage(DifficultyMode difficulty)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return 35;

        case DifficultyMode::Hard:
            return 20;

        case DifficultyMode::Lethal:
            return 10;

        case DifficultyMode::Normal:
        default:
            return 25;
    }
}

int DifficultyRules::getNonLethalDeathInventoryLossPercentage(DifficultyMode difficulty)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return 10;

        case DifficultyMode::Hard:
            return 20;

        case DifficultyMode::Normal:
        default:
            return 15;

        case DifficultyMode::Lethal:
            return 0;
    }
}

int DifficultyRules::getNonLethalDeathGoldLossPercentage(DifficultyMode difficulty)
{
    return getNonLethalDeathInventoryLossPercentage(difficulty);
}

int DifficultyRules::getNonLethalDeathExperienceLossPercentage(DifficultyMode difficulty)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return 5;

        case DifficultyMode::Hard:
            return 15;

        case DifficultyMode::Normal:
        default:
            return 10;

        case DifficultyMode::Lethal:
            return 0;
    }
}

bool DifficultyRules::isPermanentDeath(DifficultyMode difficulty)
{
    return difficulty == DifficultyMode::Lethal;
}
