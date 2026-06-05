// EN: BossPveMode.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: BossPveMode.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_MODES_BOSS_BOSSPVEMODE_HPP
#define INCLUDE_COMBAT_MODES_BOSS_BOSSPVEMODE_HPP

#include "entity/Player.hpp"
#include "core/Random.hpp"
#include "progression/DifficultyMode.hpp"
#include "progression/DeathRuleMode.hpp"
#include <vector>

class BossPveMode
{
private:
    static constexpr int BOSS_POTION_HEAL_AMOUNT = 75;
    static constexpr int BOSS_POTION_DAMAGE_BONUS = 45;

public:
    static void run(
        Player& player1,
        Random& random,
        DifficultyMode difficulty,
        DeathRuleMode deathRule
    );

    static void runTeam(
        std::vector<Player*>& party,
        Random& random,
        DifficultyMode difficulty,
        DeathRuleMode deathRule
    );
};

#endif
