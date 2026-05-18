// English: This file prepares persistent player statistics.
// Français : Ce fichier prépare les statistiques persistantes du personnage.

#include "progression/PlayerStatistics.hpp"

#include "progression/death/DeathPenaltySystem.hpp"

#include <iostream>

PlayerStatistics::PlayerStatistics()
    : deathCount(0)
{
}

int PlayerStatistics::getDeathCount() const
{
    return deathCount;
}

void PlayerStatistics::addDeath()
{
    ++deathCount;
}

void PlayerStatistics::displayDeathStatistic(DifficultyMode difficulty) const
{
    if (difficulty == DifficultyMode::Lethal)
    {
        DeathPenaltySystem::displayLethalCurrentDeathStatistic();
        return;
    }

    std::cout << "Morts du personnage : " << deathCount << std::endl;
}
