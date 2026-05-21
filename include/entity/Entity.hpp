// EN: Entity.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: Entity.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ENTITY_ENTITY_HPP
#define INCLUDE_ENTITY_ENTITY_HPP

#include <string>

#include "core/Random.hpp"
#include "class_system/PlayerClass.hpp"

class Entity
{
protected:
    std::string name;
    std::string type;

    int hp;
    int maxHp;

    int minDamage;
    int maxDamage;
    int criticalDamage;

    int healingPotionCount;
    int damagePotionCount;

    bool healingThreatMarked;
    bool provocationActive;
    int provocationTurns;

    bool defensePostureActive;
    int defenseReductionPercent;
    int defenseCounterChance;
    std::string defensePostureLabel;

public:
    // EN: Entity declares or implements a focused behavior used by this module.
    // FR: Entity déclare ou implémente un comportement précis utilisé par ce module.
    Entity();

    Entity(
        const std::string& name,
        const std::string& type,
        int maxHp,
        int minDamage,
        int maxDamage,
        int criticalDamage,
        int healingPotionCount,
        int damagePotionCount
    );

    virtual ~Entity() = default;

    std::string getName() const;
    std::string getType() const;

    // EN: getHp declares or implements a focused behavior used by this module.
    // FR: getHp déclare ou implémente un comportement précis utilisé par ce module.
    int getHp() const;
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

    // EN: isDead declares or implements a focused behavior used by this module.
    // FR: isDead déclare ou implémente un comportement précis utilisé par ce module.
    bool isDead() const;

    // EN: hasHealingThreat declares or implements a focused behavior used by this module.
    // FR: hasHealingThreat déclare ou implémente un comportement précis utilisé par ce module.
    bool hasHealingThreat() const;
    // EN: isProvoking declares or implements a focused behavior used by this module.
    // FR: isProvoking déclare ou implémente un comportement précis utilisé par ce module.
    bool isProvoking() const;
    // EN: getProvocationTurns declares or implements a focused behavior used by this module.
    // FR: getProvocationTurns déclare ou implémente un comportement précis utilisé par ce module.
    int getProvocationTurns() const;

    // EN: isInDefensePosture declares or implements a focused behavior used by this module.
    // FR: isInDefensePosture déclare ou implémente un comportement précis utilisé par ce module.
    bool isInDefensePosture() const;
    // EN: getDefenseReductionPercent declares or implements a focused behavior used by this module.
    // FR: getDefenseReductionPercent déclare ou implémente un comportement précis utilisé par ce module.
    int getDefenseReductionPercent() const;
    // EN: getDefenseCounterChance declares or implements a focused behavior used by this module.
    // FR: getDefenseCounterChance déclare ou implémente un comportement précis utilisé par ce module.
    int getDefenseCounterChance() const;
    std::string getDefensePostureLabel() const;
    // EN: startDefensePosture declares or implements a focused behavior used by this module.
    // FR: startDefensePosture déclare ou implémente un comportement précis utilisé par ce module.
    void startDefensePosture(int reductionPercent, int counterChance, const std::string& label);
    // EN: clearDefensePosture declares or implements a focused behavior used by this module.
    // FR: clearDefensePosture déclare ou implémente un comportement précis utilisé par ce module.
    void clearDefensePosture();
    // EN: markHealingThreat declares or implements a focused behavior used by this module.
    // FR: markHealingThreat déclare ou implémente un comportement précis utilisé par ce module.
    void markHealingThreat();
    // EN: clearHealingThreat declares or implements a focused behavior used by this module.
    // FR: clearHealingThreat déclare ou implémente un comportement précis utilisé par ce module.
    void clearHealingThreat();
    // EN: startProvocation declares or implements a focused behavior used by this module.
    // FR: startProvocation déclare ou implémente un comportement précis utilisé par ce module.
    void startProvocation(int turns);
    // EN: decreaseProvocationTurn declares or implements a focused behavior used by this module.
    // FR: decreaseProvocationTurn déclare ou implémente un comportement précis utilisé par ce module.
    void decreaseProvocationTurn();
    // EN: clearProvocation declares or implements a focused behavior used by this module.
    // FR: clearProvocation déclare ou implémente un comportement précis utilisé par ce module.
    void clearProvocation();

    // EN: reviveWithHealthPercentage declares or implements a focused behavior used by this module.
    // FR: reviveWithHealthPercentage déclare ou implémente un comportement précis utilisé par ce module.
    void reviveWithHealthPercentage(int percentage);

    // EN: takeDamage declares or implements a focused behavior used by this module.
    // FR: takeDamage déclare ou implémente un comportement précis utilisé par ce module.
    virtual void takeDamage(int damage);
    // EN: heal declares or implements a focused behavior used by this module.
    // FR: heal déclare ou implémente un comportement précis utilisé par ce module.
    void heal(int healAmount);
    // EN: reduceMaxHp declares or implements a focused behavior used by this module.
    // FR: reduceMaxHp déclare ou implémente un comportement précis utilisé par ce module.
    void reduceMaxHp(int value);

    virtual int attack(Random& random, bool& dodged, bool& critical, int damageBonus = 0);

    // EN: useHealingPotion declares or implements a focused behavior used by this module.
    // FR: useHealingPotion déclare ou implémente un comportement précis utilisé par ce module.
    bool useHealingPotion(int healAmount);
    // EN: consumeDamagePotion declares or implements a focused behavior used by this module.
    // FR: consumeDamagePotion déclare ou implémente un comportement précis utilisé par ce module.
    bool consumeDamagePotion();

    // EN: applyClass declares or implements a focused behavior used by this module.
    // FR: applyClass déclare ou implémente un comportement précis utilisé par ce module.
    void applyClass(const PlayerClass& newClass);

    // EN: areStatsVisible declares or implements a focused behavior used by this module.
    // FR: areStatsVisible déclare ou implémente un comportement précis utilisé par ce module.
    virtual bool areStatsVisible() const;
    // EN: displayStats declares or implements a focused behavior used by this module.
    // FR: displayStats déclare ou implémente un comportement précis utilisé par ce module.
    virtual void displayStats() const;
};

#endif
