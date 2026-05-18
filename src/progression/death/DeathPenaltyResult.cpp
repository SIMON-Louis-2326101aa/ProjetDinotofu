// English: This file prepares the future non-lethal death penalty report.
// Français : Ce fichier prépare le futur rapport de pénalité après une mort non définitive.

#include "progression/death/DeathPenaltyResult.hpp"

DeathPenaltyResult::DeathPenaltyResult()
    : lostGold(0),
      lostExperience(0),
      lostConsumables(0)
{
}

int DeathPenaltyResult::getLostGold() const
{
    return lostGold;
}

int DeathPenaltyResult::getLostExperience() const
{
    return lostExperience;
}

int DeathPenaltyResult::getLostConsumables() const
{
    return lostConsumables;
}

void DeathPenaltyResult::addLostGold(int amount)
{
    if (amount > 0)
    {
        lostGold += amount;
    }
}

void DeathPenaltyResult::addLostExperience(int amount)
{
    if (amount > 0)
    {
        lostExperience += amount;
    }
}

void DeathPenaltyResult::addLostConsumables(int amount)
{
    if (amount > 0)
    {
        lostConsumables += amount;
    }
}
