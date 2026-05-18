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
    static int getNonLethalRespawnHealthPercentage(DifficultyMode difficulty);

    static int getStarterGold(DifficultyMode difficulty);
    static int getStarterHealingPotionCount(int baseCount, DifficultyMode difficulty);
    static int getStarterDamagePotionCount(int baseCount, DifficultyMode difficulty);
    static int getStarterWeaponDurabilityLoss(DifficultyMode difficulty);
    static int getStarterArmorDurabilityLoss(DifficultyMode difficulty);

    static int getDeathEquipmentDurabilityLossPercentage(DifficultyMode difficulty);
    static int getDeathEquipmentForcedBreakChance(DifficultyMode difficulty);
    static int getDeathEquipmentIrreparableChance(DifficultyMode difficulty);
    static int getDeathWeaponTheftChance(DifficultyMode difficulty);
    static int getDeathArmorTheftChance(DifficultyMode difficulty);

    static bool isPermanentDeath(DifficultyMode difficulty);

private:
    static int clampMinimum(int value, int minimum);
};

#endif
