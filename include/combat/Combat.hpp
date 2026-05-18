// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_COMBAT_HPP
#define INCLUDE_COMBAT_COMBAT_HPP

#include "entity/Player.hpp"
#include "core/Random.hpp"
#include "progression/DifficultyMode.hpp"

class Combat
{
private:
    Random random;

public:
    void launchTwoPlayerPvp(Player& player1);
    void launchAIPvp(Player& player1);

    void launchBossPve(
        Player& player1,
        DifficultyMode difficulty
    );

    void launchMonsterPve(
        Player& player1,
        DifficultyMode difficulty
    );
};

#endif
