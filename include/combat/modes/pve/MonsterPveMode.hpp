// EN: MonsterPveMode.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: MonsterPveMode.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_MODES_PVE_MONSTERPVEMODE_HPP
#define INCLUDE_COMBAT_MODES_PVE_MONSTERPVEMODE_HPP

#include "core/Random.hpp"
#include "entity/Player.hpp"
#include "entity/Monster.hpp"
#include "progression/DifficultyMode.hpp"
#include <string>
#include <vector>

class MonsterPveMode
{
public:
    static void run(
        Player& player,
        Random& random,
        DifficultyMode difficulty
    );

    static void runTeam(
        std::vector<Player*>& party,
        Random& random,
        DifficultyMode difficulty
    );

    static bool runExplorationWave(
        Player& player,
        Random& random,
        DifficultyMode difficulty,
        const std::vector<Monster>& monsters,
        const std::string& title
    );
};

#endif
