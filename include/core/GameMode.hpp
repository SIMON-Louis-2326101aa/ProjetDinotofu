// EN: GameMode.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: GameMode.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_CORE_GAMEMODE_HPP
#define INCLUDE_CORE_GAMEMODE_HPP

enum class GameMode
{
    Story = 1,
    AIPvp = 2,
    TwoPlayerPvp = 3,
    MonsterPve = 4,
    BossPve = 5,
    Challenges = 6,
    Exploration = 7,
    Locations = 8,
    NotableNpcs = 9,
    Exchange = 10
};

#endif
