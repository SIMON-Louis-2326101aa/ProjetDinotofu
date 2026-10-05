// EN: CombatReward.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatReward.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_REWARD_COMBATREWARD_HPP
#define INCLUDE_COMBAT_REWARD_COMBATREWARD_HPP

class CombatReward
{
public:
    // EN: CombatReward declares or implements a focused behavior used by this module.
    // FR: CombatReward déclare ou implémente un comportement précis utilisé par ce module.
    CombatReward();
    CombatReward(int experience, int economyUnits);

    // EN: getExperience declares or implements a focused behavior used by this module.
    // FR: getExperience déclare ou implémente un comportement précis utilisé par ce module.
    int getExperience() const;
    // EN: Returns authored economy units (1 unit = 1 PF).
    // FR: Retourne les unités économiques de gameplay (1 unité = 1 PF).
    int getEconomyUnits() const;

    // EN: addExperience declares or implements a focused behavior used by this module.
    // FR: addExperience déclare ou implémente un comportement précis utilisé par ce module.
    void addExperience(int amount);
    // EN: Adds authored economy units (1 unit = 1 PF).
    // FR: Ajoute des unités économiques de gameplay (1 unité = 1 PF).
    void addEconomyUnits(int amount);
    // EN: addReward declares or implements a focused behavior used by this module.
    // FR: addReward déclare ou implémente un comportement précis utilisé par ce module.
    void addReward(const CombatReward& reward);

    // EN: getPercentage declares or implements a focused behavior used by this module.
    // FR: getPercentage déclare ou implémente un comportement précis utilisé par ce module.
    CombatReward getPercentage(int percentage) const;
    // EN: getModified declares or implements a focused behavior used by this module.
    // FR: getModified déclare ou implémente un comportement précis utilisé par ce module.
    CombatReward getModified(int experiencePercentage, int economyPercentage) const;

private:
    int experience;
    int economyUnits;
};

#endif
