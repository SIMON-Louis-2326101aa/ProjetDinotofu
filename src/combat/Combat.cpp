// EN: Combat.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Combat.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/Combat.hpp"

#include "combat/modes/pvp/PvpMode.hpp"
#include "combat/modes/pvp/AIPvpMode.hpp"
#include "combat/modes/boss/BossPveMode.hpp"
#include "combat/modes/pve/MonsterPveMode.hpp"

// EN: launchTwoPlayerPvp declares or implements a focused behavior used by this module.
// FR: launchTwoPlayerPvp déclare ou implémente un comportement précis utilisé par ce module.
void Combat::launchTwoPlayerPvp(Player& player1, const std::string& accountName, DifficultyMode difficulty)
{
    PvpMode::run(player1, random, accountName, difficulty);
}

// EN: launchAIPvp declares or implements a focused behavior used by this module.
// FR: launchAIPvp déclare ou implémente un comportement précis utilisé par ce module.
void Combat::launchAIPvp(Player& player1)
{
    AIPvpMode::run(player1, random);
}

void Combat::launchBossPve(
    Player& player1,
    DifficultyMode difficulty
)
{
    BossPveMode::run(player1, random, difficulty);
}

void Combat::launchMonsterPve(
    Player& player1,
    DifficultyMode difficulty
)
{
    MonsterPveMode::run(player1, random, difficulty);
}
