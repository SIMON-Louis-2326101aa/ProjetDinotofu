// EN: PlayerClass.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: PlayerClass.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "class_system/PlayerClass.hpp"

// EN: PlayerClass declares or implements a focused behavior used by this module.
// FR: PlayerClass déclare ou implémente un comportement précis utilisé par ce module.
PlayerClass::PlayerClass()
{
    name = "Aucune";
    maxHp = 100;
    minDamage = 1;
    maxDamage = 5;
    criticalDamage = 10;
    healingPotionCount = 0;
    damagePotionCount = 0;
}

PlayerClass::PlayerClass(
    const std::string& name,
    int maxHp,
    int minDamage,
    int maxDamage,
    int criticalDamage,
    int healingPotionCount,
    int damagePotionCount
)
{
    this->name = name;
    this->maxHp = maxHp;
    this->minDamage = minDamage;
    this->maxDamage = maxDamage;
    this->criticalDamage = criticalDamage;
    this->healingPotionCount = healingPotionCount;
    this->damagePotionCount = damagePotionCount;
}

std::string PlayerClass::getName() const
{
    return name;
}

// EN: getMaxHp declares or implements a focused behavior used by this module.
// FR: getMaxHp déclare ou implémente un comportement précis utilisé par ce module.
int PlayerClass::getMaxHp() const
{
    return maxHp;
}

// EN: getMinDamage declares or implements a focused behavior used by this module.
// FR: getMinDamage déclare ou implémente un comportement précis utilisé par ce module.
int PlayerClass::getMinDamage() const
{
    return minDamage;
}

// EN: getMaxDamage declares or implements a focused behavior used by this module.
// FR: getMaxDamage déclare ou implémente un comportement précis utilisé par ce module.
int PlayerClass::getMaxDamage() const
{
    return maxDamage;
}

// EN: getCriticalDamage declares or implements a focused behavior used by this module.
// FR: getCriticalDamage déclare ou implémente un comportement précis utilisé par ce module.
int PlayerClass::getCriticalDamage() const
{
    return criticalDamage;
}

// EN: getHealingPotionCount declares or implements a focused behavior used by this module.
// FR: getHealingPotionCount déclare ou implémente un comportement précis utilisé par ce module.
int PlayerClass::getHealingPotionCount() const
{
    return healingPotionCount;
}

// EN: getDamagePotionCount declares or implements a focused behavior used by this module.
// FR: getDamagePotionCount déclare ou implémente un comportement précis utilisé par ce module.
int PlayerClass::getDamagePotionCount() const
{
    return damagePotionCount;
}
