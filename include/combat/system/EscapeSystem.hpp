// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_SYSTEM_ESCAPESYSTEM_HPP
#define INCLUDE_COMBAT_SYSTEM_ESCAPESYSTEM_HPP

#include "core/Random.hpp"

#include "entity/Entity.hpp"
#include "entity/Player.hpp"
#include "entity/Monster.hpp"
#include "entity/Boss.hpp"

class EscapeSystem
{
public:
    static bool playerAttemptsEscape(Player& player, Random& random);
    static bool playerAttemptsBossEscape(const Player& player, const Boss& boss);

    static bool playerAttemptsDuelEscape(
        Player& runner,
        Entity& opponent,
        Random& random
    );

    static bool monsterCanAttemptEscape(const Monster& monster);
    static bool monsterAttemptsEscape(Monster& monster, Random& random);

private:
    static constexpr int LOW_MONSTER_ESCAPE_CHANCE = 20;

    static int calculateDuelEscapeChance(
        const Player& runner,
        const Entity& opponent
    );

    static void endCombatBySurrender(Player& runner);
};

#endif
