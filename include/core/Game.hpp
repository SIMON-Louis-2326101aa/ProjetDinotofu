// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_CORE_GAME_HPP
#define INCLUDE_CORE_GAME_HPP

#include <string>

#include "entity/Player.hpp"
#include "core/GameMode.hpp"
#include "progression/DifficultyMode.hpp"
#include "character/CharacterRace.hpp"

class Game
{
private:
    std::string accountName;
    std::string playerName;
    Player mainPlayer;
    GameMode selectedMode;
    DifficultyMode selectedDifficulty;
    CharacterRace selectedRace;
    bool characterLoaded;
    bool specialIdentityValidated;

    void displayIntroduction();
    void askAccountName();
    void askPlayerName();
    void chooseDifficulty();
    void choosePlayerRace();
    void choosePlayerClass();
    void chooseGameMode();
    void displaySelectedMode();
    void launchSelectedMode();
    bool openPostCombatMenu();
    void saveCurrentProgress(const std::string& reason) const;

    std::string getDifficultyName() const;

public:
    Game();
    void run();
};

#endif
