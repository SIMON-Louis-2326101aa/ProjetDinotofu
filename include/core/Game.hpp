// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_CORE_GAME_HPP
#define INCLUDE_CORE_GAME_HPP

#include <string>

#include "entity/Player.hpp"
#include "core/GameMode.hpp"

class Game
{
private:
    std::string playerName;
    Player mainPlayer;
    GameMode selectedMode;

    void displayIntroduction();
    void askPlayerName();
    void choosePlayerClass();
    void chooseGameMode();
    void displaySelectedMode();
    void launchSelectedMode();

public:
    Game();
    void run();
};

#endif
