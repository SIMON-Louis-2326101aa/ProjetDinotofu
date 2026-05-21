// EN: MonsterWaveCombatTurn.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: MonsterWaveCombatTurn.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_TURN_WAVE_MONSTERWAVECOMBATTURN_HPP
#define INCLUDE_COMBAT_TURN_WAVE_MONSTERWAVECOMBATTURN_HPP

#include "core/Random.hpp"

#include "entity/Player.hpp"
#include "combat/EnemyCombatQueue.hpp"
#include "combat/summon/Summon.hpp"

#include <vector>

class MonsterWaveCombatTurn
{
public:
    static void playMonsterTurns(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random
    );

    static void playMonsterTurns(
        Player& player,
        EnemyCombatQueue& wave,
        std::vector<Summon>& playerSummons,
        Random& random
    );
};

#endif
