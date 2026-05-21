// EN: PvpMode.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: PvpMode.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_MODES_PVP_PVPMODE_HPP
#define INCLUDE_COMBAT_MODES_PVP_PVPMODE_HPP

#include "entity/Player.hpp"
#include "core/Random.hpp"
#include "progression/DifficultyMode.hpp"

#include <string>

class PvpMode
{
private:
    static constexpr int POTION_HEAL_AMOUNT = 55;
    static constexpr int POTION_DAMAGE_BONUS = 25;

public:
    // EN: run declares or implements a focused behavior used by this module.
    // FR: run déclare ou implémente un comportement précis utilisé par ce module.
    static void run(Player& player1, Random& random, const std::string& account1, DifficultyMode difficulty1);
};

#endif
