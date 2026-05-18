// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_MODES_PVE_MONSTERPVEMODE_HPP
#define INCLUDE_COMBAT_MODES_PVE_MONSTERPVEMODE_HPP

#include "core/Random.hpp"
#include "entity/Player.hpp"

class MonsterPveMode
{
public:
    static void run(Player& player, Random& random);
};

#endif
