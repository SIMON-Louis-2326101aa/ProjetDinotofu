// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "class_system/PlayerClass.hpp"

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

int PlayerClass::getMaxHp() const
{
    return maxHp;
}

int PlayerClass::getMinDamage() const
{
    return minDamage;
}

int PlayerClass::getMaxDamage() const
{
    return maxDamage;
}

int PlayerClass::getCriticalDamage() const
{
    return criticalDamage;
}

int PlayerClass::getHealingPotionCount() const
{
    return healingPotionCount;
}

int PlayerClass::getDamagePotionCount() const
{
    return damagePotionCount;
}