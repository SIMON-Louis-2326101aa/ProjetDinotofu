// English: Applies the irreversible cost of surviving a lethal death through blessings.
// Français : Applique le coût irréversible d'une survie à une mort léthale grâce aux bénédictions.
#ifndef INCLUDE_PROGRESSION_BLESSING_SURVIVALANOMALY_HPP
#define INCLUDE_PROGRESSION_BLESSING_SURVIVALANOMALY_HPP

#include <string>

class Player;

class SurvivalAnomaly
{
public:
    static const std::string& permanentCurseId();
    static bool canApply(const Player& player);
    static void apply(Player& player);
};

#endif
