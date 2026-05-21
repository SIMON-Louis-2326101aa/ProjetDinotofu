// EN: EscapeSystem.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: EscapeSystem.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_SYSTEM_ESCAPESYSTEM_HPP
#define INCLUDE_COMBAT_SYSTEM_ESCAPESYSTEM_HPP

#include "core/Random.hpp"

#include "entity/Entity.hpp"
#include "entity/Player.hpp"
#include "entity/Monster.hpp"
#include "entity/Boss.hpp"
#include "progression/DifficultyMode.hpp"

class EscapeSystem
{
public:
    // EN: playerAttemptsEscape declares or implements a focused behavior used by this module.
    // FR: playerAttemptsEscape déclare ou implémente un comportement précis utilisé par ce module.
    static bool playerAttemptsEscape(Player& player, Random& random);
    static bool playerAttemptsEscape(Player& player, Random& random, DifficultyMode difficulty);
    // EN: playerAttemptsBossEscape declares or implements a focused behavior used by this module.
    // FR: playerAttemptsBossEscape déclare ou implémente un comportement précis utilisé par ce module.
    static bool playerAttemptsBossEscape(const Player& player, const Boss& boss);

    static bool playerAttemptsDuelEscape(
        Player& runner,
        Entity& opponent,
        Random& random
    );

    // EN: monsterCanAttemptEscape declares or implements a focused behavior used by this module.
    // FR: monsterCanAttemptEscape déclare ou implémente un comportement précis utilisé par ce module.
    static bool monsterCanAttemptEscape(const Monster& monster);
    // EN: monsterAttemptsEscape declares or implements a focused behavior used by this module.
    // FR: monsterAttemptsEscape déclare ou implémente un comportement précis utilisé par ce module.
    static bool monsterAttemptsEscape(Monster& monster, Random& random);

private:
    static constexpr int LOW_MONSTER_ESCAPE_CHANCE = 20;

    static int calculateDuelEscapeChance(
        const Player& runner,
        const Entity& opponent
    );

    // EN: endCombatBySurrender declares or implements a focused behavior used by this module.
    // FR: endCombatBySurrender déclare ou implémente un comportement précis utilisé par ce module.
    static void endCombatBySurrender(Player& runner);
};

#endif
