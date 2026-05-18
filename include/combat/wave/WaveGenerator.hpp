// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_WAVE_WAVEGENERATOR_HPP
#define INCLUDE_COMBAT_WAVE_WAVEGENERATOR_HPP

#include "combat/EnemyCombatQueue.hpp"
#include "core/Random.hpp"
#include "entity/Player.hpp"

class WaveGenerator
{
public:
    static EnemyCombatQueue createWaveForPlayer(
        const Player& player,
        Random& random
    );
};

#endif
