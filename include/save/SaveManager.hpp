// EN: SaveManager.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: SaveManager.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    bool clone;
    std::string path;
};

class SaveManager
{
public:
    // EN: ensureSaveDirectories declares or implements a focused behavior used by this module.
    // FR: ensureSaveDirectories déclare ou implémente un comportement précis utilisé par ce module.
    static bool ensureSaveDirectories();

    // EN: accountExists declares or implements a focused behavior used by this module.
    // FR: accountExists déclare ou implémente un comportement précis utilisé par ce module.
    static bool accountExists(const std::string& accountName);

    static bool saveAccountSnapshot(
        const std::string& accountName
    );

    static bool savePlayerSnapshot(
        const Player& player,
        const std::string& accountName,
        DifficultyMode difficulty
    );

    // EN: listAccounts declares or implements a focused behavior used by this module.
    // FR: listAccounts déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<AccountSaveSummary> listAccounts();
    // EN: listPlayableCharacters declares or implements a focused behavior used by this module.
    // FR: listPlayableCharacters déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<CharacterSaveSummary> listPlayableCharacters(const std::string& accountName);

    static bool loadPlayerSnapshot(
        const CharacterSaveSummary& summary,
        Player& player,
        DifficultyMode& difficulty
    );

    static bool movePlayableCharacterToDead(
        const std::string& accountName,
        const std::string& characterName
    );
    // EN: deletePlayableCharacter declares or implements a focused behavior used by this module.
    // FR: deletePlayableCharacter déclare ou implémente un comportement précis utilisé par ce module.
    static bool deletePlayableCharacter(const CharacterSaveSummary& summary);
    // EN: deleteAccountAndLinkedCharacters declares or implements a focused behavior used by this module.
    // FR: deleteAccountAndLinkedCharacters déclare ou implémente un comportement précis utilisé par ce module.
    static bool deleteAccountAndLinkedCharacters(const std::string& accountName);

    // EN: exportAccountPackage declares or implements a focused behavior used by this module.
    // FR: exportAccountPackage déclare ou implémente un comportement précis utilisé par ce module.
    static bool exportAccountPackage(const std::string& accountName, std::string& exportedPath);
    // EN: exportCharacterPackage declares or implements a focused behavior used by this module.
    // FR: exportCharacterPackage déclare ou implémente un comportement précis utilisé par ce module.
    static bool exportCharacterPackage(const CharacterSaveSummary& summary, std::string& exportedPath);
    // EN: exportCharacterClonePackage declares or implements a focused behavior used by this module.
    // FR: exportCharacterClonePackage déclare ou implémente un comportement précis utilisé par ce module.
    static bool exportCharacterClonePackage(const CharacterSaveSummary& summary, std::string& exportedPath);
    // EN: importAccountPackage declares or implements a focused behavior used by this module.
    // FR: importAccountPackage déclare ou implémente un comportement précis utilisé par ce module.
    static bool importAccountPackage(const std::string& packagePath, std::string& importedAccountName);

    // EN: buildSafeFileName declares or implements a focused behavior used by this module.
    // FR: buildSafeFileName déclare ou implémente un comportement précis utilisé par ce module.
    static std::string buildSafeFileName(const std::string& value);
    // EN: getAccountSavePath declares or implements a focused behavior used by this module.
    // FR: getAccountSavePath déclare ou implémente un comportement précis utilisé par ce module.
    static std::string getAccountSavePath(const std::string& accountName);
    // EN: getCharacterSavePath declares or implements a focused behavior used by this module.
    // FR: getCharacterSavePath déclare ou implémente un comportement précis utilisé par ce module.
    static std::string getCharacterSavePath(const std::string& accountName, const std::string& characterName);
};

#endif
