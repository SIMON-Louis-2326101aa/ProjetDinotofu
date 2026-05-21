// EN: DifficultyRules.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: DifficultyRules.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file prepares difficulty-dependent balancing rules.
// Français : Ce fichier prépare les règles d'équilibrage dépendantes de la difficulté.

#ifndef INCLUDE_PROGRESSION_DIFFICULTYRULES_HPP
#define INCLUDE_PROGRESSION_DIFFICULTYRULES_HPP

#include "progression/DifficultyMode.hpp"

class DifficultyRules
{
public:
    // EN: getPlayerPveEscapeDefeatedRewardPercentage declares or implements a focused behavior used by this module.
    // FR: getPlayerPveEscapeDefeatedRewardPercentage déclare ou implémente un comportement précis utilisé par ce module.
    static int getPlayerPveEscapeDefeatedRewardPercentage(DifficultyMode difficulty);
    // EN: getPlayerPveEscapeDamagedAliveRewardPercentage declares or implements a focused behavior used by this module.
    // FR: getPlayerPveEscapeDamagedAliveRewardPercentage déclare ou implémente un comportement précis utilisé par ce module.
    static int getPlayerPveEscapeDamagedAliveRewardPercentage(DifficultyMode difficulty);
    // EN: getVictoryExperienceRewardPercentage declares or implements a focused behavior used by this module.
    // FR: getVictoryExperienceRewardPercentage déclare ou implémente un comportement précis utilisé par ce module.
    static int getVictoryExperienceRewardPercentage(DifficultyMode difficulty);
    // EN: getVictoryGoldRewardPercentage declares or implements a focused behavior used by this module.
    // FR: getVictoryGoldRewardPercentage déclare ou implémente un comportement précis utilisé par ce module.
    static int getVictoryGoldRewardPercentage(DifficultyMode difficulty);
    // EN: getPlayerEscapeChanceModifier declares or implements a focused behavior used by this module.
    // FR: getPlayerEscapeChanceModifier déclare ou implémente un comportement précis utilisé par ce module.
    static int getPlayerEscapeChanceModifier(DifficultyMode difficulty);
    // EN: getMonsterHealthPercentage declares or implements a focused behavior used by this module.
    // FR: getMonsterHealthPercentage déclare ou implémente un comportement précis utilisé par ce module.
    static int getMonsterHealthPercentage(DifficultyMode difficulty);
    // EN: getMonsterDamagePercentage declares or implements a focused behavior used by this module.
    // FR: getMonsterDamagePercentage déclare ou implémente un comportement précis utilisé par ce module.
    static int getMonsterDamagePercentage(DifficultyMode difficulty);
    // EN: getEvolvedMonsterChanceModifier declares or implements a focused behavior used by this module.
    // FR: getEvolvedMonsterChanceModifier déclare ou implémente un comportement précis utilisé par ce module.
    static int getEvolvedMonsterChanceModifier(DifficultyMode difficulty);
    // EN: getLootChancePercentage declares or implements a focused behavior used by this module.
    // FR: getLootChancePercentage déclare ou implémente un comportement précis utilisé par ce module.
    static int getLootChancePercentage(DifficultyMode difficulty);
    // EN: getLootQuantityBonusChance declares or implements a focused behavior used by this module.
    // FR: getLootQuantityBonusChance déclare ou implémente un comportement précis utilisé par ce module.
    static int getLootQuantityBonusChance(DifficultyMode difficulty);

    // EN: getNonLethalDeathInventoryLossPercentage declares or implements a focused behavior used by this module.
    // FR: getNonLethalDeathInventoryLossPercentage déclare ou implémente un comportement précis utilisé par ce module.
    static int getNonLethalDeathInventoryLossPercentage(DifficultyMode difficulty);
    // EN: getNonLethalDeathGoldLossPercentage declares or implements a focused behavior used by this module.
    // FR: getNonLethalDeathGoldLossPercentage déclare ou implémente un comportement précis utilisé par ce module.
    static int getNonLethalDeathGoldLossPercentage(DifficultyMode difficulty);
    // EN: getNonLethalDeathExperienceLossPercentage declares or implements a focused behavior used by this module.
    // FR: getNonLethalDeathExperienceLossPercentage déclare ou implémente un comportement précis utilisé par ce module.
    static int getNonLethalDeathExperienceLossPercentage(DifficultyMode difficulty);
    // EN: getNonLethalRespawnHealthPercentage declares or implements a focused behavior used by this module.
    // FR: getNonLethalRespawnHealthPercentage déclare ou implémente un comportement précis utilisé par ce module.
    static int getNonLethalRespawnHealthPercentage(DifficultyMode difficulty);

    // EN: getStarterGold declares or implements a focused behavior used by this module.
    // FR: getStarterGold déclare ou implémente un comportement précis utilisé par ce module.
    static int getStarterGold(DifficultyMode difficulty);
    // EN: getStarterHealingPotionCount declares or implements a focused behavior used by this module.
    // FR: getStarterHealingPotionCount déclare ou implémente un comportement précis utilisé par ce module.
    static int getStarterHealingPotionCount(int baseCount, DifficultyMode difficulty);
    // EN: getStarterDamagePotionCount declares or implements a focused behavior used by this module.
    // FR: getStarterDamagePotionCount déclare ou implémente un comportement précis utilisé par ce module.
    static int getStarterDamagePotionCount(int baseCount, DifficultyMode difficulty);
    // EN: getStarterWeaponDurabilityLoss declares or implements a focused behavior used by this module.
    // FR: getStarterWeaponDurabilityLoss déclare ou implémente un comportement précis utilisé par ce module.
    static int getStarterWeaponDurabilityLoss(DifficultyMode difficulty);
    // EN: getStarterArmorDurabilityLoss declares or implements a focused behavior used by this module.
    // FR: getStarterArmorDurabilityLoss déclare ou implémente un comportement précis utilisé par ce module.
    static int getStarterArmorDurabilityLoss(DifficultyMode difficulty);

    // EN: getDeathEquipmentDurabilityLossPercentage declares or implements a focused behavior used by this module.
    // FR: getDeathEquipmentDurabilityLossPercentage déclare ou implémente un comportement précis utilisé par ce module.
    static int getDeathEquipmentDurabilityLossPercentage(DifficultyMode difficulty);
    // EN: getDeathEquipmentForcedBreakChance declares or implements a focused behavior used by this module.
    // FR: getDeathEquipmentForcedBreakChance déclare ou implémente un comportement précis utilisé par ce module.
    static int getDeathEquipmentForcedBreakChance(DifficultyMode difficulty);
    // EN: getDeathEquipmentIrreparableChance declares or implements a focused behavior used by this module.
    // FR: getDeathEquipmentIrreparableChance déclare ou implémente un comportement précis utilisé par ce module.
    static int getDeathEquipmentIrreparableChance(DifficultyMode difficulty);
    // EN: getDeathWeaponTheftChance declares or implements a focused behavior used by this module.
    // FR: getDeathWeaponTheftChance déclare ou implémente un comportement précis utilisé par ce module.
    static int getDeathWeaponTheftChance(DifficultyMode difficulty);
    // EN: getDeathArmorTheftChance declares or implements a focused behavior used by this module.
    // FR: getDeathArmorTheftChance déclare ou implémente un comportement précis utilisé par ce module.
    static int getDeathArmorTheftChance(DifficultyMode difficulty);

    // EN: isPermanentDeath declares or implements a focused behavior used by this module.
    // FR: isPermanentDeath déclare ou implémente un comportement précis utilisé par ce module.
    static bool isPermanentDeath(DifficultyMode difficulty);

private:
    // EN: clampMinimum declares or implements a focused behavior used by this module.
    // FR: clampMinimum déclare ou implémente un comportement précis utilisé par ce module.
    static int clampMinimum(int value, int minimum);
};

#endif
