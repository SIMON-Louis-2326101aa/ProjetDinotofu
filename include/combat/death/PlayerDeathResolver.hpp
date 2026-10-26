#pragma once

#include <string>
#include <vector>

class Player;

struct PlayerDeathImmediateResolution
{
    bool handled = false;
    bool hestiaSacrifice = false;
    std::vector<std::string> lines;
};

class PlayerDeathResolver
{
public:
    // Résout uniquement les protections qui doivent se déclencher immédiatement au moment de la chute,
    // avant les sauvetages d'équipe, bénédictions et pénalités de mort classiques.
    static PlayerDeathImmediateResolution resolveImmediate(Player& player, const std::string& context);
};
