// English: This file prepares the future non-lethal death penalty report.
// Français : Ce fichier prépare le futur rapport de pénalité après une mort non définitive.

#ifndef INCLUDE_PROGRESSION_DEATH_DEATHPENALTYRESULT_HPP
#define INCLUDE_PROGRESSION_DEATH_DEATHPENALTYRESULT_HPP

class DeathPenaltyResult
{
public:
    DeathPenaltyResult();

    int getLostGold() const;
    int getLostExperience() const;
    int getLostConsumables() const;

    void addLostGold(int amount);
    void addLostExperience(int amount);
    void addLostConsumables(int amount);

private:
    int lostGold;
    int lostExperience;
    int lostConsumables;
};

#endif
