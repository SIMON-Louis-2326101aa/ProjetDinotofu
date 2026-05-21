// EN: DifficultyProfile.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: DifficultyProfile.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Centralizes difficulty-specific percentages for combat, escape, death and rewards.

#ifndef INCLUDE_PROGRESSION_DIFFICULTY_DIFFICULTYPROFILE_HPP
#define INCLUDE_PROGRESSION_DIFFICULTY_DIFFICULTYPROFILE_HPP

#include "progression/DifficultyMode.hpp"

struct DifficultyProfile
{
    int playerEscapeChanceModifier;
    int monsterHealthPercentage;
    int monsterDamagePercentage;
    int evolvedMonsterChanceModifier;

    int playerPveEscapeDefeatedRewardPercentage;
    int playerPveEscapeDamagedAliveRewardPercentage;
    int victoryExperienceRewardPercentage;
    int victoryGoldRewardPercentage;

    int lootChancePercentage;
    int lootQuantityBonusChance;

    int nonLethalDeathInventoryLossPercentage;
    int nonLethalDeathGoldLossPercentage;
    int nonLethalDeathExperienceLossPercentage;
    int nonLethalRespawnHealthPercentage;

    int starterGold;
    int starterHealingPotionBonus;
    int starterDamagePotionBonus;
    int starterWeaponDurabilityLoss;
    int starterArmorDurabilityLoss;

    int deathEquipmentDurabilityLossPercentage;
    int deathEquipmentForcedBreakChance;
    int deathEquipmentIrreparableChance;
    int deathWeaponTheftChance;
    int deathArmorTheftChance;

    // EN: forMode declares or implements a focused behavior used by this module.
    // FR: forMode déclare ou implémente un comportement précis utilisé par ce module.
    static DifficultyProfile forMode(DifficultyMode difficulty);
};

#endif
