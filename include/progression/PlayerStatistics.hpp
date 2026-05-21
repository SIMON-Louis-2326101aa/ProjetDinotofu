// EN: PlayerStatistics.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: PlayerStatistics.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file prepares persistent player statistics.
// Français : Ce fichier prépare les statistiques persistantes du personnage.

#ifndef INCLUDE_PROGRESSION_PLAYERSTATISTICS_HPP
#define INCLUDE_PROGRESSION_PLAYERSTATISTICS_HPP

#include "progression/DifficultyMode.hpp"

class PlayerStatistics
{
public:
    // EN: PlayerStatistics declares or implements a focused behavior used by this module.
    // FR: PlayerStatistics déclare ou implémente un comportement précis utilisé par ce module.
    PlayerStatistics();

    // EN: getDeathCount declares or implements a focused behavior used by this module.
    // FR: getDeathCount déclare ou implémente un comportement précis utilisé par ce module.
    int getDeathCount() const;
    // EN: addDeath declares or implements a focused behavior used by this module.
    // FR: addDeath déclare ou implémente un comportement précis utilisé par ce module.
    void addDeath();

    // EN: displayDeathStatistic declares or implements a focused behavior used by this module.
    // FR: displayDeathStatistic déclare ou implémente un comportement précis utilisé par ce module.
    void displayDeathStatistic(DifficultyMode difficulty) const;

private:
    int deathCount;
};

#endif
