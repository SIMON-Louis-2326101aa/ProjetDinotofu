// English: This file prepares persistent player statistics.
// Français : Ce fichier prépare les statistiques persistantes du personnage.

#ifndef INCLUDE_PROGRESSION_PLAYERSTATISTICS_HPP
#define INCLUDE_PROGRESSION_PLAYERSTATISTICS_HPP

#include "progression/DifficultyMode.hpp"

class PlayerStatistics
{
public:
    PlayerStatistics();

    int getDeathCount() const;
    void addDeath();

    void displayDeathStatistic(DifficultyMode difficulty) const;

private:
    int deathCount;
};

#endif
