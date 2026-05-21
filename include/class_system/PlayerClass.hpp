// EN: PlayerClass.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: PlayerClass.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    int healingPotionCount;
    int damagePotionCount;

public:
    // EN: PlayerClass declares or implements a focused behavior used by this module.
    // FR: PlayerClass déclare ou implémente un comportement précis utilisé par ce module.
    PlayerClass();

    PlayerClass(
        const std::string& name,
        int maxHp,
        int minDamage,
        int maxDamage,
        int criticalDamage,
        int healingPotionCount,
        int damagePotionCount
    );

    std::string getName() const;
    // EN: getMaxHp declares or implements a focused behavior used by this module.
    // FR: getMaxHp déclare ou implémente un comportement précis utilisé par ce module.
    int getMaxHp() const;
    // EN: getMinDamage declares or implements a focused behavior used by this module.
    // FR: getMinDamage déclare ou implémente un comportement précis utilisé par ce module.
    int getMinDamage() const;
    // EN: getMaxDamage declares or implements a focused behavior used by this module.
    // FR: getMaxDamage déclare ou implémente un comportement précis utilisé par ce module.
    int getMaxDamage() const;
    // EN: getCriticalDamage declares or implements a focused behavior used by this module.
    // FR: getCriticalDamage déclare ou implémente un comportement précis utilisé par ce module.
    int getCriticalDamage() const;
    // EN: getHealingPotionCount declares or implements a focused behavior used by this module.
    // FR: getHealingPotionCount déclare ou implémente un comportement précis utilisé par ce module.
    int getHealingPotionCount() const;
    // EN: getDamagePotionCount declares or implements a focused behavior used by this module.
    // FR: getDamagePotionCount déclare ou implémente un comportement précis utilisé par ce module.
    int getDamagePotionCount() const;
};

#endif
