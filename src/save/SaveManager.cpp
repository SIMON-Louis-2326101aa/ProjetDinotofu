// English: This file prepares Dinotofu save helpers. Code identifiers stay in English; player-facing text can stay in French.
// Français : Ce fichier prépare les helpers de sauvegarde Dinotofu. Les identifiants restent en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Minimal JSON-like save manager for accounts, playable characters, future bestiary and materials.

#include "save/SaveManager.hpp"

#include "character/CharacterRace.hpp"
#include "class_system/ClassCatalog.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace
{
    const std::string SAVE_ROOT = "assets/saves";

    std::string escapeJson(const std::string& value)
    {
        std::string escaped;

        for (char character : value)
        {
            if (character == '"')
            {
                escaped += "\\\"";
            }
            else if (character == '\\')
            {
                escaped += "\\\\";
            }
            else
            {
                escaped += character;
            }
        }

        return escaped;
    }

    std::string readFileContent(const std::string& path)
    {
        std::ifstream file(path);

        if (!file.is_open())
        {
            return "";
        }

        std::ostringstream content;
        content << file.rdbuf();
        return content.str();
    }

    std::string extractStringValue(
        const std::string& content,
        const std::string& key,
        const std::string& fallback
    )
    {
        std::string pattern = "\"" + key + "\"";
        std::size_t keyPosition = content.find(pattern);

        if (keyPosition == std::string::npos)
        {
            return fallback;
        }

        std::size_t colonPosition = content.find(':', keyPosition);

        if (colonPosition == std::string::npos)
        {
            return fallback;
        }

        std::size_t firstQuote = content.find('"', colonPosition + 1);

        if (firstQuote == std::string::npos)
        {
            return fallback;
        }

        std::size_t secondQuote = content.find('"', firstQuote + 1);

        if (secondQuote == std::string::npos)
        {
            return fallback;
        }

        return content.substr(firstQuote + 1, secondQuote - firstQuote - 1);
    }

    int extractIntValue(
        const std::string& content,
        const std::string& key,
        int fallback
    )
    {
        std::string pattern = "\"" + key + "\"";
        std::size_t keyPosition = content.find(pattern);

        if (keyPosition == std::string::npos)
        {
            return fallback;
        }

        std::size_t colonPosition = content.find(':', keyPosition);

        if (colonPosition == std::string::npos)
        {
            return fallback;
        }

        std::size_t start = colonPosition + 1;

        while (start < content.size() && std::isspace(static_cast<unsigned char>(content[start])))
        {
            start++;
        }

        std::size_t end = start;

        if (end < content.size() && content[end] == '-')
        {
            end++;
        }

        while (end < content.size() && std::isdigit(static_cast<unsigned char>(content[end])))
        {
            end++;
        }

        if (end == start)
        {
            return fallback;
        }

        try
        {
            return std::stoi(content.substr(start, end - start));
        }
        catch (...)
        {
            return fallback;
        }
    }

    std::string normalizeText(const std::string& value)
    {
        std::string normalized;

        for (unsigned char character : value)
        {
            if (std::isalnum(character))
            {
                normalized += static_cast<char>(std::tolower(character));
            }
        }

        return normalized;
    }

    DifficultyMode difficultyFromText(const std::string& value)
    {
        std::string normalized = normalizeText(value);

        if (normalized == "facile" || normalized == "easy")
        {
            return DifficultyMode::Easy;
        }

        if (normalized == "difficile" || normalized == "hard")
        {
            return DifficultyMode::Hard;
        }

        if (normalized == "cauchemar" || normalized == "nightmare")
        {
            return DifficultyMode::Nightmare;
        }

        if (normalized == "lethal" || normalized == "lthal" || normalized == "mortel")
        {
            return DifficultyMode::Lethal;
        }

        return DifficultyMode::Normal;
    }

    std::string difficultyToText(DifficultyMode difficulty)
    {
        switch (difficulty)
        {
            case DifficultyMode::Easy:
                return "Facile";

            case DifficultyMode::Hard:
                return "Difficile";

            case DifficultyMode::Nightmare:
                return "Cauchemar";

            case DifficultyMode::Lethal:
                return "Léthal";

            case DifficultyMode::Normal:
            default:
                return "Normal";
        }
    }

    CharacterRace raceFromText(const std::string& value)
    {
        std::vector<CharacterRace> races = {
            CharacterRace::Human,
            CharacterRace::Elf,
            CharacterRace::DarkElf,
            CharacterRace::Dwarf,
            CharacterRace::Gnome,
            CharacterRace::Halfling,
            CharacterRace::Tiefling,
            CharacterRace::Aasimar,
            CharacterRace::Kitsune,
            CharacterRace::Fairy,
            CharacterRace::HalfDragon,
            CharacterRace::Orc,
            CharacterRace::Vampire,
            CharacterRace::Demon,
            CharacterRace::SemiHuman,
            CharacterRace::Other
        };

        std::string normalized = normalizeText(value);

        for (CharacterRace race : races)
        {
            if (normalizeText(characterRaceToText(race)) == normalized)
            {
                return race;
            }
        }

        return CharacterRace::Human;
    }

    CharacterSaveSummary parseCharacterSummary(const std::string& path)
    {
        std::string content = readFileContent(path);

        CharacterSaveSummary summary;
        summary.path = path;
        summary.accountName = extractStringValue(content, "account", "local");
        summary.characterName = extractStringValue(content, "name", "Inconnu");
        summary.raceName = extractStringValue(content, "race", "Humain");
        summary.className = extractStringValue(content, "class", "Chevalier");
        summary.difficulty = difficultyFromText(extractStringValue(content, "difficulty", "Normal"));
        summary.level = extractIntValue(content, "level", 1);

        return summary;
    }
}

bool SaveManager::ensureSaveDirectories()
{
    try
    {
        std::filesystem::create_directories(SAVE_ROOT + "/accounts");
        std::filesystem::create_directories(SAVE_ROOT + "/characters/playable");
        std::filesystem::create_directories(SAVE_ROOT + "/characters/dead");
        std::filesystem::create_directories(SAVE_ROOT + "/bestiary");
        std::filesystem::create_directories(SAVE_ROOT + "/materials");
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool SaveManager::accountExists(const std::string& accountName)
{
    return std::filesystem::exists(getAccountSavePath(accountName));
}

bool SaveManager::saveAccountSnapshot(
    const std::string& accountName
)
{
    if (!ensureSaveDirectories())
    {
        return false;
    }

    std::ofstream file(getAccountSavePath(accountName));

    if (!file.is_open())
    {
        return false;
    }

    file << "{\n";
    file << "  \"saveVersion\": 2,\n";
    file << "  \"accountName\": \"" << escapeJson(accountName) << "\",\n";
    file << "  \"status\": \"active\",\n";
    file << "  \"saveFolders\": {\n";
    file << "    \"playableCharacters\": \"assets/saves/characters/playable\",\n";
    file << "    \"deadCharacters\": \"assets/saves/characters/dead\",\n";
    file << "    \"bestiary\": \"assets/saves/bestiary\",\n";
    file << "    \"materials\": \"assets/saves/materials\"\n";
    file << "  },\n";
    file << "  \"futureData\": {\n";
    file << "    \"globalStatistics\": \"prepared\",\n";
    file << "    \"deadCharacters\": \"prepared\",\n";
    file << "    \"bestiary\": \"prepared\",\n";
    file << "    \"materials\": \"prepared\"\n";
    file << "  }\n";
    file << "}\n";

    return true;
}

bool SaveManager::savePlayerSnapshot(
    const Player& player,
    const std::string& accountName,
    DifficultyMode difficulty
)
{
    if (!ensureSaveDirectories())
    {
        return false;
    }

    std::ofstream file(getCharacterSavePath(accountName, player.getName()));

    if (!file.is_open())
    {
        return false;
    }

    Weapon weapon = player.getEquippedWeapon();
    Armor armor = player.getEquippedArmor();

    file << "{\n";
    file << "  \"saveVersion\": 2,\n";
    file << "  \"account\": \"" << escapeJson(accountName) << "\",\n";
    file << "  \"difficulty\": \"" << escapeJson(difficultyToText(difficulty)) << "\",\n";
    file << "  \"character\": {\n";
    file << "    \"name\": \"" << escapeJson(player.getName()) << "\",\n";
    file << "    \"race\": \"" << escapeJson(player.getRaceText()) << "\",\n";
    file << "    \"class\": \"" << escapeJson(player.getType()) << "\",\n";
    file << "    \"level\": " << player.getLevel() << ",\n";
    file << "    \"experience\": " << player.getExperience() << ",\n";
    file << "    \"hp\": " << player.getHp() << ",\n";
    file << "    \"maxHp\": " << player.getMaxHp() << ",\n";
    file << "    \"gold\": " << player.getInventory().getGold() << ",\n";
    file << "    \"equippedWeaponIndex\": " << player.getEquippedWeaponIndex() << ",\n";
    file << "    \"equippedWeaponName\": \"" << escapeJson(weapon.getName()) << "\",\n";
    file << "    \"equippedArmorIndex\": " << player.getEquippedArmorIndex() << ",\n";
    file << "    \"equippedArmorName\": \"" << escapeJson(armor.getName()) << "\"\n";
    file << "  },\n";
    file << "  \"futureData\": {\n";
    file << "    \"inventoryFullSerialization\": \"prepared\",\n";
    file << "    \"bestiary\": \"prepared\",\n";
    file << "    \"materials\": \"prepared\",\n";
    file << "    \"cheats\": \"prepared\",\n";
    file << "    \"blessings\": \"prepared\"\n";
    file << "  }\n";
    file << "}\n";

    return true;
}

std::vector<AccountSaveSummary> SaveManager::listAccounts()
{
    ensureSaveDirectories();

    std::vector<AccountSaveSummary> accounts;
    std::string accountFolder = SAVE_ROOT + "/accounts";

    try
    {
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(accountFolder))
        {
            if (!entry.is_regular_file() || entry.path().extension() != ".json")
            {
                continue;
            }

            std::string path = entry.path().string();
            std::string content = readFileContent(path);

            AccountSaveSummary summary;
            summary.path = path;
            summary.accountName = extractStringValue(content, "accountName", entry.path().stem().string());

            accounts.push_back(summary);
        }
    }
    catch (...)
    {
        return accounts;
    }

    std::sort(accounts.begin(), accounts.end(), [](const AccountSaveSummary& a, const AccountSaveSummary& b) {
        return a.accountName < b.accountName;
    });

    return accounts;
}

std::vector<CharacterSaveSummary> SaveManager::listPlayableCharacters(const std::string& accountName)
{
    ensureSaveDirectories();

    std::vector<CharacterSaveSummary> characters;
    std::string characterFolder = SAVE_ROOT + "/characters/playable";
    std::string safeAccount = buildSafeFileName(accountName) + "__";

    try
    {
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(characterFolder))
        {
            if (!entry.is_regular_file() || entry.path().extension() != ".json")
            {
                continue;
            }

            std::string fileName = entry.path().filename().string();

            if (fileName.rfind(safeAccount, 0) != 0)
            {
                continue;
            }

            characters.push_back(parseCharacterSummary(entry.path().string()));
        }
    }
    catch (...)
    {
        return characters;
    }

    std::sort(characters.begin(), characters.end(), [](const CharacterSaveSummary& a, const CharacterSaveSummary& b) {
        return a.characterName < b.characterName;
    });

    return characters;
}

bool SaveManager::loadPlayerSnapshot(
    const CharacterSaveSummary& summary,
    Player& player,
    DifficultyMode& difficulty
)
{
    std::string content = readFileContent(summary.path);

    if (content.empty())
    {
        return false;
    }

    std::string characterName = extractStringValue(content, "name", summary.characterName);
    std::string className = extractStringValue(content, "class", summary.className);
    std::string raceName = extractStringValue(content, "race", summary.raceName);

    int level = extractIntValue(content, "level", 1);
    int experience = extractIntValue(content, "experience", 0);
    int hp = extractIntValue(content, "hp", 1);
    int gold = extractIntValue(content, "gold", 0);
    int equippedWeaponIndex = extractIntValue(content, "equippedWeaponIndex", -1);
    int equippedArmorIndex = extractIntValue(content, "equippedArmorIndex", -1);

    difficulty = difficultyFromText(extractStringValue(content, "difficulty", "Normal"));

    PlayerClass loadedClass = ClassCatalog::createClassByName(className);
    player = Player(characterName, loadedClass);
    player.setRace(raceFromText(raceName));
    player.initializeStarterInventory(difficulty);
    player.getInventory().setGold(gold);
    player.setLoadedProgress(level, experience, hp);

    if (equippedWeaponIndex >= 0)
    {
        player.equipWeapon(equippedWeaponIndex);
    }

    if (equippedArmorIndex >= 0)
    {
        player.equipArmor(equippedArmorIndex);
    }

    return true;
}

std::string SaveManager::buildSafeFileName(const std::string& value)
{
    std::string safe;

    for (unsigned char character : value)
    {
        if (std::isalnum(character))
        {
            safe += static_cast<char>(std::tolower(character));
        }
        else if (character == '-' || character == '_')
        {
            safe += static_cast<char>(character);
        }
        else if (character == ' ')
        {
            safe += '-';
        }
    }

    if (safe.empty())
    {
        return "unknown";
    }

    return safe;
}

std::string SaveManager::getAccountSavePath(const std::string& accountName)
{
    return SAVE_ROOT
        + "/accounts/"
        + buildSafeFileName(accountName)
        + ".json";
}

std::string SaveManager::getCharacterSavePath(
    const std::string& accountName,
    const std::string& characterName
)
{
    return SAVE_ROOT
        + "/characters/playable/"
        + buildSafeFileName(accountName)
        + "__"
        + buildSafeFileName(characterName)
        + ".json";
}
