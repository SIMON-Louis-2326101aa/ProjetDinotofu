// EN: CombatReward.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatReward.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/reward/CombatReward.hpp"

// EN: CombatReward declares or implements a focused behavior used by this module.
// FR: CombatReward déclare ou implémente un comportement précis utilisé par ce module.
CombatReward::CombatReward()
    : experience(0),
      // EN: gold declares or implements a focused behavior used by this module.
      // FR: gold déclare ou implémente un comportement précis utilisé par ce module.
      gold(0)
{
}

// EN: CombatReward declares or implements a focused behavior used by this module.
// FR: CombatReward déclare ou implémente un comportement précis utilisé par ce module.
CombatReward::CombatReward(int experience, int gold)
    : experience(experience),
      // EN: gold declares or implements a focused behavior used by this module.
      // FR: gold déclare ou implémente un comportement précis utilisé par ce module.
      gold(gold)
{
}

// EN: getExperience declares or implements a focused behavior used by this module.
// FR: getExperience déclare ou implémente un comportement précis utilisé par ce module.
int CombatReward::getExperience() const
{
    return experience;
}

// EN: getGold declares or implements a focused behavior used by this module.
// FR: getGold déclare ou implémente un comportement précis utilisé par ce module.
int CombatReward::getGold() const
{
    return gold;
}

// EN: addExperience declares or implements a focused behavior used by this module.
// FR: addExperience déclare ou implémente un comportement précis utilisé par ce module.
void CombatReward::addExperience(int amount)
{
    if (amount > 0)
    {
        experience += amount;
    }
}

// EN: addGold declares or implements a focused behavior used by this module.
// FR: addGold déclare ou implémente un comportement précis utilisé par ce module.
void CombatReward::addGold(int amount)
{
    if (amount > 0)
    {
        gold += amount;
    }
}

// EN: addReward declares or implements a focused behavior used by this module.
// FR: addReward déclare ou implémente un comportement précis utilisé par ce module.
void CombatReward::addReward(const CombatReward& reward)
{
    addExperience(reward.getExperience());
    addGold(reward.getGold());
}

// EN: getPercentage declares or implements a focused behavior used by this module.
// FR: getPercentage déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: getModified declares or implements a focused behavior used by this module.
// FR: getModified déclare ou implémente un comportement précis utilisé par ce module.
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
