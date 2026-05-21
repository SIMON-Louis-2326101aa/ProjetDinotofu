// EN: PlayerWaveCombatTurn.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: PlayerWaveCombatTurn.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_TURN_WAVE_PLAYERWAVECOMBATTURN_HPP
#define INCLUDE_COMBAT_TURN_WAVE_PLAYERWAVECOMBATTURN_HPP

#include "core/Random.hpp"

#include "entity/Player.hpp"
#include "combat/EnemyCombatQueue.hpp"
#include "progression/DifficultyMode.hpp"

class PlayerWaveCombatTurn
{
public:
    static bool play(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random,
        bool& escapeSucceeded,
        DifficultyMode difficulty
    );

private:
    static constexpr int PVE_POTION_DAMAGE_BONUS = 25;

    static bool openWaveInterface(
        Player& player,
        EnemyCombatQueue& wave,
        DifficultyMode difficulty
    );
};

#endif
