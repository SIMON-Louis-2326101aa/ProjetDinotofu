// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_MODES_PVP_PVPMODE_HPP
#define INCLUDE_COMBAT_MODES_PVP_PVPMODE_HPP

#include "entity/Player.hpp"
#include "core/Random.hpp"

class PvpMode
{
private:
    static constexpr int SOIN_POTION = 55;
    static constexpr int BONUS_POTION_DEGATS = 25;

public:
    static void run(Player& player1, Random& random);
};

#endif
