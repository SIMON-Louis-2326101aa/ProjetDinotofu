// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_CLASS_SYSTEM_PLAYERCLASS_HPP
#define INCLUDE_CLASS_SYSTEM_PLAYERCLASS_HPP

#include <string>

class PlayerClass
{
private:
    std::string name;
    int maxHp;
    int minDamage;
    int maxDamage;
    int criticalDamage;
    int healingPotions;
    int damagePotions;

public:
    PlayerClass();

    PlayerClass(
        const std::string& name,
        int maxHp,
        int minDamage,
        int maxDamage,
        int criticalDamage,
        int healingPotions,
        int damagePotions
    );

    std::string getName() const;
    int getMaxHp() const;
    int getMinDamage() const;
    int getMaxDamage() const;
    int getCriticalDamage() const;
    int getHealingPotions() const;
    int getDamagePotions() const;
};

#endif
