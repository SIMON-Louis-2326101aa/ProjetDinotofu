// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/Combat.hpp"

#include "combat/modes/pvp/PvpMode.hpp"
#include "combat/modes/pvp/AIPvpMode.hpp"
#include "combat/modes/boss/BossPveMode.hpp"
#include "combat/modes/pve/MonsterPveMode.hpp"

void Combat::launchTwoPlayerPvp(Player& player1)
{
    PvpMode::run(player1, random);
}

void Combat::launchAIPvp(Player& player1)
{
    AIPvpMode::run(player1, random);
}

void Combat::launchBossPve(Player& player1)
{
    BossPveMode::run(player1, random);
}

void Combat::launchMonsterPve(Player& player1)
{
    MonsterPveMode::run(player1, random);
}