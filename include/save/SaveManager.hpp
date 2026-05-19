// English: This file prepares Dinotofu save helpers. Code identifiers stay in English; player-facing text can stay in French.
// Français : Ce fichier prépare les helpers de sauvegarde Dinotofu. Les identifiants restent en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Minimal JSON-like save manager for accounts, playable characters, future bestiary and materials.

#ifndef INCLUDE_SAVE_SAVEMANAGER_HPP
#define INCLUDE_SAVE_SAVEMANAGER_HPP

#include "entity/Player.hpp"
#include "progression/DifficultyMode.hpp"

#include <string>
#include <vector>

struct AccountSaveSummary
{
    std::string accountName;
    std::string path;
};

struct CharacterSaveSummary
{
    std::string accountName;
    std::string characterName;
    std::string raceName;
    std::string className;
    DifficultyMode difficulty;
    int level;
    std::string path;
};

class SaveManager
{
public:
    static bool ensureSaveDirectories();

    static bool accountExists(const std::string& accountName);

    static bool saveAccountSnapshot(
        const std::string& accountName
    );

    static bool savePlayerSnapshot(
        const Player& player,
        const std::string& accountName,
        DifficultyMode difficulty
    );

    static std::vector<AccountSaveSummary> listAccounts();
    static std::vector<CharacterSaveSummary> listPlayableCharacters(const std::string& accountName);

    static bool loadPlayerSnapshot(
        const CharacterSaveSummary& summary,
        Player& player,
        DifficultyMode& difficulty
    );

    static std::string buildSafeFileName(const std::string& value);
    static std::string getAccountSavePath(const std::string& accountName);
    static std::string getCharacterSavePath(const std::string& accountName, const std::string& characterName);
};

#endif
