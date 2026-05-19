// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/reward/CombatReward.hpp"

CombatReward::CombatReward()
    : experience(0),
      gold(0)
{
}

CombatReward::CombatReward(int experience, int gold)
    : experience(experience),
      gold(gold)
{
}

int CombatReward::getExperience() const
{
    return experience;
}

int CombatReward::getGold() const
{
    return gold;
}

void CombatReward::addExperience(int amount)
{
    if (amount > 0)
    {
        experience += amount;
    }
}

void CombatReward::addGold(int amount)
{
    if (amount > 0)
    {
        gold += amount;
    }
}

void CombatReward::addReward(const CombatReward& reward)
{
    addExperience(reward.getExperience());
    addGold(reward.getGold());
}

CombatReward CombatReward::getPercentage(int percentage) const
{
    if (percentage <= 0)
    {
        return CombatReward();
    }

    if (percentage >= 100)
    {
        return *this;
    }

    return CombatReward(
        experience * percentage / 100,
        gold * percentage / 100
    );
}

CombatReward CombatReward::getModified(int experiencePercentage, int goldPercentage) const
{
    if (experiencePercentage < 0)
    {
        experiencePercentage = 0;
    }

    if (goldPercentage < 0)
    {
        goldPercentage = 0;
    }

    return CombatReward(
        experience * experiencePercentage / 100,
        gold * goldPercentage / 100
    );
}
