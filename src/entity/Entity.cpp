// EN: Entity.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Entity.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "entity/Entity.hpp"

#include <algorithm>

#include <iostream>

// EN: Entity declares or implements a focused behavior used by this module.
// FR: Entity déclare ou implémente un comportement précis utilisé par ce module.
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

    healingThreatMarked = false;
    provocationActive = false;
    provocationTurns = 0;
    defensePostureActive = false;
    defenseReductionPercent = 0;
    defenseCounterChance = 0;
    defensePostureLabel = "";
    burningTurns = 0;
    burningDamage = 0;
    poisonTurns = 0;
    poisonDamage = 0;
    frostTurns = 0;
    shockTurns = 0;
    bleedingTurns = 0;
    bleedingDamage = 0;
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

    healingThreatMarked = false;
    provocationActive = false;
    provocationTurns = 0;
    defensePostureActive = false;
    defenseReductionPercent = 0;
    defenseCounterChance = 0;
    defensePostureLabel = "";
    burningTurns = 0;
    burningDamage = 0;
    poisonTurns = 0;
    poisonDamage = 0;
    frostTurns = 0;
    shockTurns = 0;
    bleedingTurns = 0;
    bleedingDamage = 0;
}

std::string Entity::getName() const
{
    return name;
}

std::string Entity::getType() const
{
    return type;
}

// EN: getHp declares or implements a focused behavior used by this module.
// FR: getHp déclare ou implémente un comportement précis utilisé par ce module.
int Entity::getHp() const
{
    return hp;
}

// EN: getMaxHp declares or implements a focused behavior used by this module.
// FR: getMaxHp déclare ou implémente un comportement précis utilisé par ce module.
int Entity::getMaxHp() const
{
    return maxHp;
}

// EN: getMinDamage declares or implements a focused behavior used by this module.
// FR: getMinDamage déclare ou implémente un comportement précis utilisé par ce module.
int Entity::getMinDamage() const
{
    return minDamage;
}

// EN: getMaxDamage declares or implements a focused behavior used by this module.
// FR: getMaxDamage déclare ou implémente un comportement précis utilisé par ce module.
int Entity::getMaxDamage() const
{
    return maxDamage;
}

// EN: getCriticalDamage declares or implements a focused behavior used by this module.
// FR: getCriticalDamage déclare ou implémente un comportement précis utilisé par ce module.
int Entity::getCriticalDamage() const
{
    return criticalDamage;
}

// EN: getHealingPotionCount declares or implements a focused behavior used by this module.
// FR: getHealingPotionCount déclare ou implémente un comportement précis utilisé par ce module.
int Entity::getHealingPotionCount() const
{
    return healingPotionCount;
}

// EN: getDamagePotionCount declares or implements a focused behavior used by this module.
// FR: getDamagePotionCount déclare ou implémente un comportement précis utilisé par ce module.
int Entity::getDamagePotionCount() const
{
    return damagePotionCount;
}

// EN: isDead declares or implements a focused behavior used by this module.
// FR: isDead déclare ou implémente un comportement précis utilisé par ce module.
bool Entity::isDead() const
{
    return hp <= 0;
}

// EN: hasHealingThreat declares or implements a focused behavior used by this module.
// FR: hasHealingThreat déclare ou implémente un comportement précis utilisé par ce module.
bool Entity::hasHealingThreat() const
{
    return healingThreatMarked;
}

// EN: isProvoking declares or implements a focused behavior used by this module.
// FR: isProvoking déclare ou implémente un comportement précis utilisé par ce module.
bool Entity::isProvoking() const
{
    return provocationActive && provocationTurns > 0;
}

// EN: getProvocationTurns declares or implements a focused behavior used by this module.
// FR: getProvocationTurns déclare ou implémente un comportement précis utilisé par ce module.
int Entity::getProvocationTurns() const
{
    return provocationTurns;
}

// EN: markHealingThreat declares or implements a focused behavior used by this module.
// FR: markHealingThreat déclare ou implémente un comportement précis utilisé par ce module.
void Entity::markHealingThreat()
{
    healingThreatMarked = true;
}

// EN: clearHealingThreat declares or implements a focused behavior used by this module.
// FR: clearHealingThreat déclare ou implémente un comportement précis utilisé par ce module.
void Entity::clearHealingThreat()
{
    healingThreatMarked = false;
}

// EN: startProvocation declares or implements a focused behavior used by this module.
// FR: startProvocation déclare ou implémente un comportement précis utilisé par ce module.
void Entity::startProvocation(int turns)
{
    if (turns <= 0)
    {
        return;
    }

    provocationActive = true;
    provocationTurns = turns;
}

// EN: decreaseProvocationTurn declares or implements a focused behavior used by this module.
// FR: decreaseProvocationTurn déclare ou implémente un comportement précis utilisé par ce module.
void Entity::decreaseProvocationTurn()
{
    if (provocationTurns > 0)
    {
        provocationTurns--;
    }

    if (provocationTurns <= 0)
    {
        clearProvocation();
    }
}

// EN: clearProvocation declares or implements a focused behavior used by this module.
// FR: clearProvocation déclare ou implémente un comportement précis utilisé par ce module.
void Entity::clearProvocation()
{
    provocationActive = false;
    provocationTurns = 0;
}


// EN: isInDefensePosture declares or implements a focused behavior used by this module.
// FR: isInDefensePosture déclare ou implémente un comportement précis utilisé par ce module.
bool Entity::isInDefensePosture() const
{
    return defensePostureActive;
}

// EN: getDefenseReductionPercent declares or implements a focused behavior used by this module.
// FR: getDefenseReductionPercent déclare ou implémente un comportement précis utilisé par ce module.
int Entity::getDefenseReductionPercent() const
{
    return defenseReductionPercent;
}

// EN: getDefenseCounterChance declares or implements a focused behavior used by this module.
// FR: getDefenseCounterChance déclare ou implémente un comportement précis utilisé par ce module.
int Entity::getDefenseCounterChance() const
{
    return defenseCounterChance;
}

std::string Entity::getDefensePostureLabel() const
{
    return defensePostureLabel;
}

// EN: startDefensePosture declares or implements a focused behavior used by this module.
// FR: startDefensePosture déclare ou implémente un comportement précis utilisé par ce module.
void Entity::startDefensePosture(int reductionPercent, int counterChance, const std::string& label)
{
    if (reductionPercent < 0) reductionPercent = 0;
    if (reductionPercent > 80) reductionPercent = 80;
    if (counterChance < 0) counterChance = 0;
    if (counterChance > 80) counterChance = 80;

    defensePostureActive = true;
    defenseReductionPercent = reductionPercent;
    defenseCounterChance = counterChance;
    defensePostureLabel = label;
}

// EN: clearDefensePosture declares or implements a focused behavior used by this module.
// FR: clearDefensePosture déclare ou implémente un comportement précis utilisé par ce module.
void Entity::clearDefensePosture()
{
    defensePostureActive = false;
    defenseReductionPercent = 0;
    defenseCounterChance = 0;
    defensePostureLabel = "";
    burningTurns = 0;
    burningDamage = 0;
    poisonTurns = 0;
    poisonDamage = 0;
    frostTurns = 0;
    shockTurns = 0;
    bleedingTurns = 0;
    bleedingDamage = 0;
}


void Entity::applyBurning(int turns, int damage)
{
    if (turns <= 0 || damage <= 0) return;
    burningTurns = std::max(burningTurns, turns);
    burningDamage = std::max(burningDamage, damage);
}

void Entity::applyPoison(int turns, int damage)
{
    if (turns <= 0 || damage <= 0) return;
    poisonTurns = std::max(poisonTurns, turns);
    poisonDamage = std::max(poisonDamage, damage);
}

void Entity::applyFrost(int turns)
{
    if (turns <= 0) return;
    frostTurns = std::max(frostTurns, turns);
}

void Entity::applyShock(int turns)
{
    if (turns <= 0) return;
    shockTurns = std::max(shockTurns, turns);
}

void Entity::applyBleeding(int turns, int damage)
{
    if (turns <= 0 || damage <= 0) return;
    bleedingTurns = std::max(bleedingTurns, turns);
    bleedingDamage = std::max(bleedingDamage, damage);
}

bool Entity::hasActiveCombatStatus() const
{
    return burningTurns > 0 || poisonTurns > 0 || frostTurns > 0 || shockTurns > 0 || bleedingTurns > 0;
}

void Entity::processStatusTickAtTurnStart()
{
    if (hp <= 0) return;

    int totalDamage = 0;

    if (burningTurns > 0)
    {
        totalDamage += burningDamage;
        std::cout << name << " subit une brûlure persistante (" << burningDamage << " dégâts)." << std::endl;
        burningTurns--;
        if (burningTurns <= 0)
        {
            burningDamage = 0;
            std::cout << "La brûlure de " << name << " s'éteint." << std::endl;
        }
    }

    if (poisonTurns > 0)
    {
        totalDamage += poisonDamage;
        std::cout << name << " subit le poison (" << poisonDamage << " dégâts)." << std::endl;
        poisonTurns--;
        if (poisonTurns <= 0)
        {
            poisonDamage = 0;
            std::cout << "Le poison quitte enfin le corps de " << name << "." << std::endl;
        }
    }

    if (bleedingTurns > 0)
    {
        totalDamage += bleedingDamage;
        std::cout << name << " perd du sang (" << bleedingDamage << " dégâts)." << std::endl;
        bleedingTurns--;
        if (bleedingTurns <= 0)
        {
            bleedingDamage = 0;
            std::cout << "Le saignement de " << name << " se calme." << std::endl;
        }
    }

    if (frostTurns > 0)
    {
        std::cout << name << " reste ralenti par le froid." << std::endl;
        frostTurns--;
        if (frostTurns <= 0)
        {
            std::cout << name << " retrouve une mobilité normale." << std::endl;
        }
    }

    if (shockTurns > 0)
    {
        std::cout << "Des arcs électriques perturbent encore " << name << "." << std::endl;
        shockTurns--;
        if (shockTurns <= 0)
        {
            std::cout << "L'électricité autour de " << name << " se dissipe." << std::endl;
        }
    }

    if (totalDamage > 0)
    {
        takeDamage(totalDamage);
        std::cout << name << " possède maintenant " << hp << "/" << maxHp << " PV après les statuts." << std::endl;
        std::cout << std::endl;
    }
}

// EN: reviveWithHealthPercentage declares or implements a focused behavior used by this module.
// FR: reviveWithHealthPercentage déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: takeDamage declares or implements a focused behavior used by this module.
// FR: takeDamage déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: heal declares or implements a focused behavior used by this module.
// FR: heal déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: reduceMaxHp declares or implements a focused behavior used by this module.
// FR: reduceMaxHp déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: attack declares or implements a focused behavior used by this module.
// FR: attack déclare ou implémente un comportement précis utilisé par ce module.

void Entity::scaleCombatStats(int hpPercent, int damagePercent)
{
    if (hpPercent < 1) hpPercent = 1;
    if (damagePercent < 1) damagePercent = 1;

    int oldMaxHp = maxHp;
    maxHp = std::max(1, maxHp * hpPercent / 100);
    minDamage = std::max(1, minDamage * damagePercent / 100);
    maxDamage = std::max(minDamage, maxDamage * damagePercent / 100);
    criticalDamage = std::max(maxDamage, criticalDamage * damagePercent / 100);

    if (oldMaxHp > 0)
    {
        hp = std::max(1, hp * maxHp / oldMaxHp);
    }

    if (hp > maxHp) hp = maxHp;
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

// EN: useHealingPotion declares or implements a focused behavior used by this module.
// FR: useHealingPotion déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: consumeDamagePotion declares or implements a focused behavior used by this module.
// FR: consumeDamagePotion déclare ou implémente un comportement précis utilisé par ce module.
bool Entity::consumeDamagePotion()
{
    if (damagePotionCount <= 0)
    {
        return false;
    }

    damagePotionCount--;
    return true;
}

// EN: applyClass declares or implements a focused behavior used by this module.
// FR: applyClass déclare ou implémente un comportement précis utilisé par ce module.
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

void Entity::restoreClassState(const PlayerClass& baseClass, int currentHp)
{
    type = baseClass.getName();

    maxHp = baseClass.getMaxHp();
    minDamage = baseClass.getMinDamage();
    maxDamage = baseClass.getMaxDamage();
    criticalDamage = baseClass.getCriticalDamage();

    healingPotionCount = baseClass.getHealingPotionCount();
    damagePotionCount = baseClass.getDamagePotionCount();

    if (currentHp < 0)
    {
        currentHp = 0;
    }

    hp = currentHp;

    if (hp > maxHp)
    {
        hp = maxHp;
    }
}

// EN: areStatsVisible declares or implements a focused behavior used by this module.
// FR: areStatsVisible déclare ou implémente un comportement précis utilisé par ce module.
bool Entity::areStatsVisible() const
{
    return true;
}

// EN: displayStats declares or implements a focused behavior used by this module.
// FR: displayStats déclare ou implémente un comportement précis utilisé par ce module.
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
