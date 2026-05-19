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

public:
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

    int getHp() const;
    int getMaxHp() const;

    int getMinDamage() const;
    int getMaxDamage() const;
    int getCriticalDamage() const;

    int getHealingPotionCount() const;
    int getDamagePotionCount() const;

    bool isDead() const;

    bool hasHealingThreat() const;
    bool isProvoking() const;
    int getProvocationTurns() const;
    void markHealingThreat();
    void clearHealingThreat();
    void startProvocation(int turns);
    void decreaseProvocationTurn();
    void clearProvocation();

    void reviveWithHealthPercentage(int percentage);

    virtual void takeDamage(int damage);
    void heal(int healAmount);
    void reduceMaxHp(int value);

    virtual int attack(Random& random, bool& dodged, bool& critical, int damageBonus = 0);

    bool useHealingPotion(int healAmount);
    bool consumeDamagePotion();

    void applyClass(const PlayerClass& newClass);

    virtual bool areStatsVisible() const;
    virtual void displayStats() const;
};

#endif
