// EN: DifficultyRules.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: DifficultyRules.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file prepares difficulty-dependent balancing rules.
// Français : Ce fichier prépare les règles d'équilibrage dépendantes de la difficulté.

#include "progression/DifficultyRules.hpp"
#include "progression/difficulty/DifficultyProfile.hpp"

// EN: getPlayerPveEscapeDefeatedRewardPercentage declares or implements a focused behavior used by this module.
// FR: getPlayerPveEscapeDefeatedRewardPercentage déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getPlayerPveEscapeDefeatedRewardPercentage(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).playerPveEscapeDefeatedRewardPercentage;
}

// EN: getPlayerPveEscapeDamagedAliveRewardPercentage declares or implements a focused behavior used by this module.
// FR: getPlayerPveEscapeDamagedAliveRewardPercentage déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getPlayerPveEscapeDamagedAliveRewardPercentage(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).playerPveEscapeDamagedAliveRewardPercentage;
}

// EN: getVictoryExperienceRewardPercentage declares or implements a focused behavior used by this module.
// FR: getVictoryExperienceRewardPercentage déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getVictoryExperienceRewardPercentage(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).victoryExperienceRewardPercentage;
}

// EN: getVictoryGoldRewardPercentage declares or implements a focused behavior used by this module.
// FR: getVictoryGoldRewardPercentage déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getVictoryGoldRewardPercentage(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).victoryGoldRewardPercentage;
}

// EN: getPlayerEscapeChanceModifier declares or implements a focused behavior used by this module.
// FR: getPlayerEscapeChanceModifier déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getPlayerEscapeChanceModifier(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).playerEscapeChanceModifier;
}

// EN: getMonsterHealthPercentage declares or implements a focused behavior used by this module.
// FR: getMonsterHealthPercentage déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getMonsterHealthPercentage(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).monsterHealthPercentage;
}

// EN: getMonsterDamagePercentage declares or implements a focused behavior used by this module.
// FR: getMonsterDamagePercentage déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getMonsterDamagePercentage(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).monsterDamagePercentage;
}

// EN: getEvolvedMonsterChanceModifier declares or implements a focused behavior used by this module.
// FR: getEvolvedMonsterChanceModifier déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getEvolvedMonsterChanceModifier(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).evolvedMonsterChanceModifier;
}

// EN: getLootChancePercentage declares or implements a focused behavior used by this module.
// FR: getLootChancePercentage déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getLootChancePercentage(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).lootChancePercentage;
}

// EN: getLootQuantityBonusChance declares or implements a focused behavior used by this module.
// FR: getLootQuantityBonusChance déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getLootQuantityBonusChance(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).lootQuantityBonusChance;
}

// EN: getNonLethalDeathInventoryLossPercentage declares or implements a focused behavior used by this module.
// FR: getNonLethalDeathInventoryLossPercentage déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getNonLethalDeathInventoryLossPercentage(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).nonLethalDeathInventoryLossPercentage;
}

// EN: getNonLethalDeathGoldLossPercentage declares or implements a focused behavior used by this module.
// FR: getNonLethalDeathGoldLossPercentage déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getNonLethalDeathGoldLossPercentage(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).nonLethalDeathGoldLossPercentage;
}

// EN: getNonLethalDeathExperienceLossPercentage declares or implements a focused behavior used by this module.
// FR: getNonLethalDeathExperienceLossPercentage déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getNonLethalDeathExperienceLossPercentage(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).nonLethalDeathExperienceLossPercentage;
}

// EN: getNonLethalRespawnHealthPercentage declares or implements a focused behavior used by this module.
// FR: getNonLethalRespawnHealthPercentage déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getNonLethalRespawnHealthPercentage(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).nonLethalRespawnHealthPercentage;
}

// EN: getStarterGold declares or implements a focused behavior used by this module.
// FR: getStarterGold déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getStarterGold(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).starterGold;
}

// EN: getStarterHealingPotionCount declares or implements a focused behavior used by this module.
// FR: getStarterHealingPotionCount déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getStarterHealingPotionCount(int baseCount, DifficultyMode difficulty)
{
    return clampMinimum(baseCount + DifficultyProfile::forMode(difficulty).starterHealingPotionBonus, 0);
}

// EN: getStarterDamagePotionCount declares or implements a focused behavior used by this module.
// FR: getStarterDamagePotionCount déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getStarterDamagePotionCount(int baseCount, DifficultyMode difficulty)
{
    return clampMinimum(baseCount + DifficultyProfile::forMode(difficulty).starterDamagePotionBonus, 0);
}

// EN: getStarterWeaponDurabilityLoss declares or implements a focused behavior used by this module.
// FR: getStarterWeaponDurabilityLoss déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getStarterWeaponDurabilityLoss(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).starterWeaponDurabilityLoss;
}

// EN: getStarterArmorDurabilityLoss declares or implements a focused behavior used by this module.
// FR: getStarterArmorDurabilityLoss déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getStarterArmorDurabilityLoss(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).starterArmorDurabilityLoss;
}

// EN: getDeathEquipmentDurabilityLossPercentage declares or implements a focused behavior used by this module.
// FR: getDeathEquipmentDurabilityLossPercentage déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getDeathEquipmentDurabilityLossPercentage(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).deathEquipmentDurabilityLossPercentage;
}

// EN: getDeathEquipmentForcedBreakChance declares or implements a focused behavior used by this module.
// FR: getDeathEquipmentForcedBreakChance déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getDeathEquipmentForcedBreakChance(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).deathEquipmentForcedBreakChance;
}

// EN: getDeathEquipmentIrreparableChance declares or implements a focused behavior used by this module.
// FR: getDeathEquipmentIrreparableChance déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getDeathEquipmentIrreparableChance(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).deathEquipmentIrreparableChance;
}

// EN: getDeathWeaponTheftChance declares or implements a focused behavior used by this module.
// FR: getDeathWeaponTheftChance déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getDeathWeaponTheftChance(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).deathWeaponTheftChance;
}

// EN: getDeathArmorTheftChance declares or implements a focused behavior used by this module.
// FR: getDeathArmorTheftChance déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::getDeathArmorTheftChance(DifficultyMode difficulty)
{
    return DifficultyProfile::forMode(difficulty).deathArmorTheftChance;
}

// EN: isPermanentDeath declares or implements a focused behavior used by this module.
// FR: isPermanentDeath déclare ou implémente un comportement précis utilisé par ce module.
bool DifficultyRules::isPermanentDeath(DifficultyMode difficulty)
{
    return difficulty == DifficultyMode::Lethal;
}

// EN: clampMinimum declares or implements a focused behavior used by this module.
// FR: clampMinimum déclare ou implémente un comportement précis utilisé par ce module.
int DifficultyRules::clampMinimum(int value, int minimum)
{
    if (value < minimum)
    {
        return minimum;
    }

    return value;
}
