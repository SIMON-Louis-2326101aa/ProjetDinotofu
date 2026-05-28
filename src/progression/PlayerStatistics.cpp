// EN: PlayerStatistics.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: PlayerStatistics.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file prepares persistent player statistics.
// Français : Ce fichier prépare les statistiques persistantes du personnage.

#include "progression/PlayerStatistics.hpp"

#include "progression/death/DeathPenaltySystem.hpp"

#include "interface/menu/common/MessageScreen.hpp"

#include <iostream>
// EN: PlayerStatistics declares or implements a focused behavior used by this module.
// FR: PlayerStatistics déclare ou implémente un comportement précis utilisé par ce module.
PlayerStatistics::PlayerStatistics()
    // EN: deathCount declares or implements a focused behavior used by this module.
    // FR: deathCount déclare ou implémente un comportement précis utilisé par ce module.
    : deathCount(0)
{
}

// EN: getDeathCount declares or implements a focused behavior used by this module.
// FR: getDeathCount déclare ou implémente un comportement précis utilisé par ce module.
int PlayerStatistics::getDeathCount() const
{
    return deathCount;
}

// EN: addDeath declares or implements a focused behavior used by this module.
// FR: addDeath déclare ou implémente un comportement précis utilisé par ce module.
void PlayerStatistics::addDeath()
{
    ++deathCount;
}

// EN: displayDeathStatistic declares or implements a focused behavior used by this module.
// FR: displayDeathStatistic déclare ou implémente un comportement précis utilisé par ce module.
void PlayerStatistics::displayDeathStatistic(DifficultyMode difficulty) const
{
    if (difficulty == DifficultyMode::Lethal)
    {
        DeathPenaltySystem::displayLethalCurrentDeathStatistic();
        return;
    }

    MessageScreen::show(
        "STATISTIQUE DE MORT",
        "player.statistics.death",
        {"Morts du personnage : " + std::to_string(deathCount)},
        false
    );
}
