// EN: Summon.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Summon.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Stores basic summoned combat entity data for future 1v1-to-PvE transitions.

#include "combat/summon/Summon.hpp"

// EN: Summon declares or implements a focused behavior used by this module.
// FR: Summon déclare ou implémente un comportement précis utilisé par ce module.
Summon::Summon()
{
    name = "Invocation inconnue";
    ownerName = "Inconnu";
    hp = 1;
    maxHp = 1;
    minDamage = 0;
    maxDamage = 1;
    durationTurns = 1;
    slotCost = 1;
    maintenanceCost = 1;
    sacrificial = true;
}

Summon::Summon(
    const std::string& name,
    const std::string& ownerName,
    int maxHp,
    int minDamage,
    int maxDamage,
    int durationTurns,
    int slotCost,
    int maintenanceCost,
    bool sacrificial
)
{
    this->name = name;
    this->ownerName = ownerName;
    this->maxHp = maxHp > 0 ? maxHp : 1;
    this->hp = this->maxHp;
    this->minDamage = minDamage >= 0 ? minDamage : 0;
    this->maxDamage = maxDamage >= this->minDamage ? maxDamage : this->minDamage;
    this->durationTurns = durationTurns > 0 ? durationTurns : 1;
    this->slotCost = slotCost > 0 ? slotCost : 1;
    this->maintenanceCost = maintenanceCost > 0 ? maintenanceCost : 1;
    this->sacrificial = sacrificial;
}

std::string Summon::getName() const
{
    return name;
}

std::string Summon::getOwnerName() const
{
    return ownerName;
}

// EN: getHp declares or implements a focused behavior used by this module.
// FR: getHp déclare ou implémente un comportement précis utilisé par ce module.
int Summon::getHp() const
{
    return hp;
}

// EN: getMaxHp declares or implements a focused behavior used by this module.
// FR: getMaxHp déclare ou implémente un comportement précis utilisé par ce module.
int Summon::getMaxHp() const
{
    return maxHp;
}

// EN: getMinDamage declares or implements a focused behavior used by this module.
// FR: getMinDamage déclare ou implémente un comportement précis utilisé par ce module.
int Summon::getMinDamage() const
{
    return minDamage;
}

// EN: getMaxDamage declares or implements a focused behavior used by this module.
// FR: getMaxDamage déclare ou implémente un comportement précis utilisé par ce module.
int Summon::getMaxDamage() const
{
    return maxDamage;
}

// EN: getDurationTurns declares or implements a focused behavior used by this module.
// FR: getDurationTurns déclare ou implémente un comportement précis utilisé par ce module.
int Summon::getDurationTurns() const
{
    return durationTurns;
}

// EN: getSlotCost declares or implements a focused behavior used by this module.
// FR: getSlotCost déclare ou implémente un comportement précis utilisé par ce module.
int Summon::getSlotCost() const
{
    return slotCost;
}

// EN: getMaintenanceCost declares or implements a focused behavior used by this module.
// FR: getMaintenanceCost déclare ou implémente un comportement précis utilisé par ce module.
int Summon::getMaintenanceCost() const
{
    return maintenanceCost;
}

// EN: canBeSacrificed declares or implements a focused behavior used by this module.
// FR: canBeSacrificed déclare ou implémente un comportement précis utilisé par ce module.
bool Summon::canBeSacrificed() const
{
    return sacrificial;
}

// EN: isDead declares or implements a focused behavior used by this module.
// FR: isDead déclare ou implémente un comportement précis utilisé par ce module.
bool Summon::isDead() const
{
    return hp <= 0;
}

// EN: isExpired declares or implements a focused behavior used by this module.
// FR: isExpired déclare ou implémente un comportement précis utilisé par ce module.
bool Summon::isExpired() const
{
    return durationTurns <= 0;
}

// EN: takeDamage declares or implements a focused behavior used by this module.
// FR: takeDamage déclare ou implémente un comportement précis utilisé par ce module.
void Summon::takeDamage(int damage)
{
    if (damage <= 0)
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
void Summon::heal(int healAmount)
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

// EN: setDurationTurns declares or implements a focused behavior used by this module.
// FR: setDurationTurns déclare ou implémente un comportement précis utilisé par ce module.
void Summon::setDurationTurns(int turns)
{
    durationTurns = turns < 0 ? 0 : turns;
}

// EN: decreaseDuration declares or implements a focused behavior used by this module.
// FR: decreaseDuration déclare ou implémente un comportement précis utilisé par ce module.
void Summon::decreaseDuration()
{
    durationTurns--;

    if (durationTurns < 0)
    {
        durationTurns = 0;
    }
}

// EN: extendDuration declares or implements a focused behavior used by this module.
// FR: extendDuration déclare ou implémente un comportement précis utilisé par ce module.
void Summon::extendDuration(int turns)
{
    if (turns <= 0)
    {
        return;
    }

    durationTurns += turns;
}

// EN: markSacrificed declares or implements a focused behavior used by this module.
// FR: markSacrificed déclare ou implémente un comportement précis utilisé par ce module.
void Summon::markSacrificed()
{
    hp = 0;
    durationTurns = 0;
}
