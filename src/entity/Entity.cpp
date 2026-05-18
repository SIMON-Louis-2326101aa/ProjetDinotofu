// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "entity/Entity.hpp"

#include <iostream>

Entity::Entity()
{
    name = "Inconnu";
    type = "Aucun";

    hp = 100;
    maxHp = 100;

    minDamage = 1;
    maxDamage = 5;
    criticalDamage = 10;

    healingPotionCount = 0;
    damagePotionCount = 0;
}

Entity::Entity(
    const std::string& name,
    const std::string& type,
    int maxHp,
    int minDamage,
    int maxDamage,
    int criticalDamage,
    int healingPotionCount,
    int damagePotionCount
)
{
    this->name = name;
    this->type = type;

    this->hp = maxHp;
    this->maxHp = maxHp;

    this->minDamage = minDamage;
    this->maxDamage = maxDamage;
    this->criticalDamage = criticalDamage;

    this->healingPotionCount = healingPotionCount;
    this->damagePotionCount = damagePotionCount;
}

std::string Entity::getName() const
{
    return name;
}

std::string Entity::getType() const
{
    return type;
}

int Entity::getHp() const
{
    return hp;
}

int Entity::getMaxHp() const
{
    return maxHp;
}

int Entity::getMinDamage() const
{
    return minDamage;
}

int Entity::getMaxDamage() const
{
    return maxDamage;
}

int Entity::getCriticalDamage() const
{
    return criticalDamage;
}

int Entity::getHealingPotionCount() const
{
    return healingPotionCount;
}

int Entity::getDamagePotionCount() const
{
    return damagePotionCount;
}

bool Entity::isDead() const
{
    return hp <= 0;
}

void Entity::reviveWithHealthPercentage(int percentage)
{
    if (percentage <= 0)
    {
        percentage = 1;
    }

    if (percentage > 100)
    {
        percentage = 100;
    }

    int restoredHp = maxHp * percentage / 100;

    if (restoredHp <= 0)
    {
        restoredHp = 1;
    }

    hp = restoredHp;
}

void Entity::takeDamage(int damage)
{
    if (damage < 0)
    {
        return;
    }

    hp -= damage;

    if (hp < 0)
    {
        hp = 0;
    }
}

void Entity::heal(int healAmount)
{
    if (healAmount <= 0)
    {
        return;
    }

    hp += healAmount;

    if (hp > maxHp)
    {
        hp = maxHp;
    }
}

void Entity::reduceMaxHp(int value)
{
    if (value <= 0)
    {
        return;
    }

    maxHp -= value;

    if (maxHp < 1)
    {
        maxHp = 1;
    }

    if (hp > maxHp)
    {
        hp = maxHp;
    }
}

int Entity::attack(Random& random, bool& dodged, bool& critical, int damageBonus)
{
    int resultat = random.rollD20();

    dodged = false;
    critical = false;

    if (resultat <= 3)
    {
        dodged = true;
        return 0;
    }

    if (resultat <= 16)
    {
        return random.between(minDamage, maxDamage) + damageBonus;
    }

    critical = true;
    return criticalDamage + damageBonus;
}

bool Entity::useHealingPotion(int healAmount)
{
    if (healingPotionCount <= 0)
    {
        return false;
    }

    healingPotionCount--;
    heal(healAmount);

    return true;
}

bool Entity::consumeDamagePotion()
{
    if (damagePotionCount <= 0)
    {
        return false;
    }

    damagePotionCount--;
    return true;
}

void Entity::applyClass(const PlayerClass& newClass)
{
    type = newClass.getName();

    maxHp = newClass.getMaxHp();
    hp = maxHp;

    minDamage = newClass.getMinDamage();
    maxDamage = newClass.getMaxDamage();
    criticalDamage = newClass.getCriticalDamage();

    healingPotionCount = newClass.getHealingPotionCount();
    damagePotionCount = newClass.getDamagePotionCount();
}

bool Entity::areStatsVisible() const
{
    return true;
}

void Entity::displayStats() const
{
    std::cout << name << std::endl;
    std::cout << "Type : " << type << std::endl;
    std::cout << "PV : " << hp << "/" << maxHp << std::endl;
    std::cout << "Dégâts min : " << minDamage << std::endl;
    std::cout << "Dégâts max : " << maxDamage << std::endl;
    std::cout << "Dégâts crit : " << criticalDamage << std::endl;
    std::cout << "Potions de soin : " << healingPotionCount << std::endl;
    std::cout << "Potions de dégâts : " << damagePotionCount << std::endl;
    std::cout << std::endl;
}