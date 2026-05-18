// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/BossCombat.hpp"

#include "combat/boss/BossDecryption.hpp"
#include "combat/boss/BossUltimate.hpp"
#include "combat/boss/BossEndTurn.hpp"

void BossCombat::checkBossDecryption(Boss& boss)
{
    BossDecryption::checkBossDecryption(boss);
}

void BossCombat::executeBossUltimate(
    Boss& boss,
    Entity& player,
    Random& random
)
{
    BossUltimate::executeBossUltimate(
        boss,
        player,
        random
    );
}

bool BossCombat::handleBossEndTurn(
    Boss& boss,
    Entity& player
)
{
    return BossEndTurn::handleBossEndTurn(
        boss,
        player
    );
}