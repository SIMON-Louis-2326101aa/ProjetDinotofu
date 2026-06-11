// English: Consumes blessings to resolve exceptional lethal-survival effects.
// Français : Consume les bénédictions pour résoudre les survies exceptionnelles en mode léthal.
#ifndef INCLUDE_PROGRESSION_BLESSING_BLESSINGSYSTEM_HPP
#define INCLUDE_PROGRESSION_BLESSING_BLESSINGSYSTEM_HPP

class Player;

class BlessingSystem
{
public:
    static bool tryTriggerLethalSurvival(Player& player);
};

#endif
