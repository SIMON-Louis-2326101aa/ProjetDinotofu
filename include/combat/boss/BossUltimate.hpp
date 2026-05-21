// EN: BossUltimate.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: BossUltimate.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_BOSS_BOSSULTIMATE_HPP
#define INCLUDE_COMBAT_BOSS_BOSSULTIMATE_HPP

#include "core/Random.hpp"

#include "entity/Boss.hpp"
#include "entity/Entity.hpp"

class BossUltimate
{
public:
    static void executeBossUltimate(
        Boss& boss,
        Entity& player,
        Random& random
    );
};

#endif
