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

        case DifficultyMode::Nightmare:
            return 25;

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

        case DifficultyMode::Nightmare:
            return 10;

        case DifficultyMode::Lethal:
            return 10;

        case DifficultyMode::Normal:
        default:
            return 25;
    }
}


int DifficultyRules::getVictoryExperienceRewardPercentage(DifficultyMode difficulty)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return 90;

        case DifficultyMode::Hard:
            return 115;

        case DifficultyMode::Nightmare:
            return 130;

        case DifficultyMode::Lethal:
            return 150;

        case DifficultyMode::Normal:
        default:
            return 100;
    }
}

int DifficultyRules::getVictoryGoldRewardPercentage(DifficultyMode difficulty)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return 110;

        case DifficultyMode::Hard:
            return 90;

        case DifficultyMode::Nightmare:
            return 75;

        case DifficultyMode::Lethal:
            return 65;

        case DifficultyMode::Normal:
        default:
            return 100;
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

        case DifficultyMode::Nightmare:
            return 25;

        case DifficultyMode::Lethal:
            return 0;

        case DifficultyMode::Normal:
        default:
            return 15;
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

        case DifficultyMode::Nightmare:
            return 20;

        case DifficultyMode::Lethal:
            return 0;

        case DifficultyMode::Normal:
        default:
            return 10;
    }
}

int DifficultyRules::getNonLethalRespawnHealthPercentage(DifficultyMode difficulty)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return 75;

        case DifficultyMode::Hard:
            return 30;

        case DifficultyMode::Nightmare:
            return 10;

        case DifficultyMode::Lethal:
            return 1;

        case DifficultyMode::Normal:
        default:
            return 50;
    }
}

int DifficultyRules::getStarterGold(DifficultyMode difficulty)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return 75;

        case DifficultyMode::Hard:
            return 30;

        case DifficultyMode::Nightmare:
            return 20;

        case DifficultyMode::Lethal:
            return 15;

        case DifficultyMode::Normal:
        default:
            return 50;
    }
}

int DifficultyRules::getStarterHealingPotionCount(
    int baseCount,
    DifficultyMode difficulty
)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return baseCount + 1;

        case DifficultyMode::Hard:
            return clampMinimum(baseCount - 1, 0);

        case DifficultyMode::Nightmare:
            return clampMinimum(baseCount - 2, 0);

        case DifficultyMode::Lethal:
            return clampMinimum(baseCount - 2, 0);

        case DifficultyMode::Normal:
        default:
            return baseCount;
    }
}

int DifficultyRules::getStarterDamagePotionCount(
    int baseCount,
    DifficultyMode difficulty
)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return baseCount;

        case DifficultyMode::Hard:
            return clampMinimum(baseCount - 1, 0);

        case DifficultyMode::Nightmare:
            return clampMinimum(baseCount - 1, 0);

        case DifficultyMode::Lethal:
            return clampMinimum(baseCount - 1, 0);

        case DifficultyMode::Normal:
        default:
            return baseCount;
    }
}

int DifficultyRules::getStarterWeaponDurabilityLoss(DifficultyMode difficulty)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return 0;

        case DifficultyMode::Hard:
            return 20;

        case DifficultyMode::Nightmare:
            return 30;

        case DifficultyMode::Lethal:
            return 35;

        case DifficultyMode::Normal:
        default:
            return 0;
    }
}

int DifficultyRules::getStarterArmorDurabilityLoss(DifficultyMode difficulty)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return 0;

        case DifficultyMode::Hard:
            return 25;

        case DifficultyMode::Nightmare:
            return 35;

        case DifficultyMode::Lethal:
            return 45;

        case DifficultyMode::Normal:
        default:
            return 0;
    }
}

int DifficultyRules::getDeathEquipmentDurabilityLossPercentage(DifficultyMode difficulty)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return 35;

        case DifficultyMode::Hard:
            return 55;

        case DifficultyMode::Nightmare:
            return 65;

        case DifficultyMode::Lethal:
            return 100;

        case DifficultyMode::Normal:
        default:
            return 50;
    }
}

int DifficultyRules::getDeathEquipmentForcedBreakChance(DifficultyMode difficulty)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return 10;

        case DifficultyMode::Hard:
            return 25;

        case DifficultyMode::Nightmare:
            return 30;

        case DifficultyMode::Lethal:
            return 40;

        case DifficultyMode::Normal:
        default:
            return 20;
    }
}

int DifficultyRules::getDeathEquipmentIrreparableChance(DifficultyMode difficulty)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return 5;

        case DifficultyMode::Hard:
            return 12;

        case DifficultyMode::Nightmare:
            return 18;

        case DifficultyMode::Lethal:
            return 25;

        case DifficultyMode::Normal:
        default:
            return 10;
    }
}

int DifficultyRules::getDeathWeaponTheftChance(DifficultyMode difficulty)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return 2;

        case DifficultyMode::Hard:
            return 5;

        case DifficultyMode::Nightmare:
            return 7;

        case DifficultyMode::Lethal:
            return 9;

        case DifficultyMode::Normal:
        default:
            return 4;
    }
}

int DifficultyRules::getDeathArmorTheftChance(DifficultyMode difficulty)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return 1;

        case DifficultyMode::Hard:
            return 3;

        case DifficultyMode::Nightmare:
            return 5;

        case DifficultyMode::Lethal:
            return 7;

        case DifficultyMode::Normal:
        default:
            return 2;
    }
}

bool DifficultyRules::isPermanentDeath(DifficultyMode difficulty)
{
    return difficulty == DifficultyMode::Lethal;
}

int DifficultyRules::clampMinimum(int value, int minimum)
{
    if (value < minimum)
    {
        return minimum;
    }

    return value;
}
