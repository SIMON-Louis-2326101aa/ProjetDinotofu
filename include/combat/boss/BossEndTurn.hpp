// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_BOSS_BOSSENDTURN_HPP
#define INCLUDE_COMBAT_BOSS_BOSSENDTURN_HPP

#include "entity/Boss.hpp"
#include "entity/Entity.hpp"

class BossEndTurn
{
public:
    static bool handleBossEndTurn(
        Boss& boss,
        Entity& player
    );
};

#endif
