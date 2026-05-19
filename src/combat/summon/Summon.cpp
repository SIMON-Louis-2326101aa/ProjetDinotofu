// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Stores basic summoned combat entity data for future 1v1-to-PvE transitions.

#include "combat/summon/Summon.hpp"

Summon::Summon()
{
    name = "Invocation inconnue";
    ownerName = "Inconnu";
    hp = 1;
    maxHp = 1;
    minDamage = 0;
    maxDamage = 1;
    durationTurns = 1;
}

Summon::Summon(
    const std::string& name,
    const std::string& ownerName,
    int maxHp,
    int minDamage,
    int maxDamage,
    int durationTurns
)
{
    this->name = name;
    this->ownerName = ownerName;
    this->maxHp = maxHp > 0 ? maxHp : 1;
    this->hp = this->maxHp;
    this->minDamage = minDamage >= 0 ? minDamage : 0;
    this->maxDamage = maxDamage >= this->minDamage ? maxDamage : this->minDamage;
    this->durationTurns = durationTurns > 0 ? durationTurns : 1;
}

std::string Summon::getName() const
{
    return name;
}

std::string Summon::getOwnerName() const
{
    return ownerName;
}

int Summon::getHp() const
{
    return hp;
}

int Summon::getMaxHp() const
{
    return maxHp;
}

int Summon::getMinDamage() const
{
    return minDamage;
}

int Summon::getMaxDamage() const
{
    return maxDamage;
}

int Summon::getDurationTurns() const
{
    return durationTurns;
}

bool Summon::isDead() const
{
    return hp <= 0;
}

bool Summon::isExpired() const
{
    return durationTurns <= 0;
}

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

void Summon::decreaseDuration()
{
    durationTurns--;

    if (durationTurns < 0)
    {
        durationTurns = 0;
    }
}
