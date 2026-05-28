// EN: Game.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: Game.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_CORE_GAME_HPP
#define INCLUDE_CORE_GAME_HPP

#include <string>
#include <vector>

#include "entity/Player.hpp"
#include "core/GameMode.hpp"
#include "progression/DifficultyMode.hpp"
#include "character/CharacterRace.hpp"
#include "interface/model/MenuScreen.hpp"

class Game
{
private:
    struct CombatRecapSnapshot
    {
        int level = 0;
        int experience = 0;
        int hp = 0;
        int maxHp = 0;
        int gold = 0;
        int victories = 0;
        int defeats = 0;
        int escapes = 0;
        int enemiesKilled = 0;
        int bossesKilled = 0;
    };

    struct LastCombatRecap
    {
        bool available = false;
        std::string modeName;
        std::string difficultyName;
        CombatRecapSnapshot before;
        CombatRecapSnapshot after;
    };

    std::string accountName;
    std::string playerName;
    Player mainPlayer;
    GameMode selectedMode;
    DifficultyMode selectedDifficulty;
    CharacterRace selectedRace;
    bool characterLoaded;
    bool specialIdentityValidated;
    std::vector<Player> partyPlayers;
    std::vector<std::string> partyAccountNames;
    std::vector<DifficultyMode> partyDifficulties;
    LastCombatRecap lastCombatRecap;

    // EN: displayIntroduction declares or implements a focused behavior used by this module.
    // FR: displayIntroduction déclare ou implémente un comportement précis utilisé par ce module.
    void displayIntroduction();
    // EN: askAccountName declares or implements a focused behavior used by this module.
    // FR: askAccountName déclare ou implémente un comportement précis utilisé par ce module.
    void askAccountName();
    // EN: askPlayerName declares or implements a focused behavior used by this module.
    // FR: askPlayerName déclare ou implémente un comportement précis utilisé par ce module.
    void askPlayerName();
    // EN: chooseDifficulty declares or implements a focused behavior used by this module.
    // FR: chooseDifficulty déclare ou implémente un comportement précis utilisé par ce module.
    void chooseDifficulty();
    // EN: choosePlayerRace declares or implements a focused behavior used by this module.
    // FR: choosePlayerRace déclare ou implémente un comportement précis utilisé par ce module.
    void choosePlayerRace();
    // EN: choosePlayerClass declares or implements a focused behavior used by this module.
    // FR: choosePlayerClass déclare ou implémente un comportement précis utilisé par ce module.
    void choosePlayerClass();
    // EN: chooseGameMode declares or implements a focused behavior used by this module.
    // FR: chooseGameMode déclare ou implémente un comportement précis utilisé par ce module.
    void chooseGameMode();
    void configurePartyMode();
    bool addSecondaryPlayerToParty(int playerNumber);
    bool isMultiplayerSession() const;
    std::vector<Player*> getActivePartyPointers();
    void savePartyProgress(const std::string& reason) const;
    // EN: displaySelectedMode declares or implements a focused behavior used by this module.
    // FR: displaySelectedMode déclare ou implémente un comportement précis utilisé par ce module.
    void displaySelectedMode();
    // EN: displayActivityInformation declares or implements a focused behavior used by this module.
    // FR: displayActivityInformation déclare ou implémente un comportement précis utilisé par ce module.
    void displayActivityInformation() const;
    std::string getSelectedModeName() const;
    CombatRecapSnapshot captureCombatRecapSnapshot() const;
    void updateLastCombatRecap(const CombatRecapSnapshot& beforeSnapshot);
    void displayLastCombatRecap() const;
    // EN: launchSelectedMode declares or implements a focused behavior used by this module.
    // FR: launchSelectedMode déclare ou implémente un comportement précis utilisé par ce module.
    void launchSelectedMode();
    // EN: launchStoryModePlaceholder declares or implements a focused behavior used by this module.
    // FR: launchStoryModePlaceholder déclare ou implémente un comportement précis utilisé par ce module.
    void launchStoryModePlaceholder();
    // EN: launchChallengeBoard declares or implements a focused behavior used by this module.
    // FR: launchChallengeBoard déclare ou implémente un comportement précis utilisé par ce module.
    void launchChallengeBoard();
    // EN: openPostCombatMenu declares or implements a focused behavior used by this module.
    // FR: openPostCombatMenu déclare ou implémente un comportement précis utilisé par ce module.
    bool openPostCombatMenu();
    void addOutOfCombatUtilityOptions(MenuScreen& screen, bool inventoryAvailable = true, bool saveAvailable = true) const;
    bool handleOutOfCombatUtilityChoice(int choice, bool inventoryAvailable = true);
    void openGuardianInputMenu();
    // EN: openExchangeMenu declares or implements a focused behavior used by this module.
    // FR: openExchangeMenu déclare ou implémente un comportement précis utilisé par ce module.
    void openExchangeMenu();
    // EN: saveCurrentProgress declares or implements a focused behavior used by this module.
    // FR: saveCurrentProgress déclare ou implémente un comportement précis utilisé par ce module.
    void saveCurrentProgress(const std::string& reason) const;

    std::string getDifficultyName() const;

public:
    // EN: Game declares or implements a focused behavior used by this module.
    // FR: Game déclare ou implémente un comportement précis utilisé par ce module.
    Game();
    // EN: run declares or implements a focused behavior used by this module.
    // FR: run déclare ou implémente un comportement précis utilisé par ce module.
    void run();
};

#endif
