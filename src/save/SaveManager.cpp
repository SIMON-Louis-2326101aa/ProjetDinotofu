// English: This file prepares Dinotofu save helpers. Code identifiers stay in English; player-facing text can stay in French.
// Français : Ce fichier prépare les helpers de sauvegarde Dinotofu. Les identifiants restent en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Minimal JSON-like save manager for accounts, playable characters, future bestiary and materials.

#include "save/SaveManager.hpp"

#include "character/CharacterRace.hpp"
#include "class_system/ClassCatalog.hpp"
#include "item/armor/ArmorCatalog.hpp"
#include "item/consumable/ConsumableCatalog.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "item/weapon/WeaponCatalog.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace
{
    std::string normalizeText(const std::string& value);

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

    bool extractBoolValue(
        const std::string& content,
        const std::string& key,
        bool fallback
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

        if (content.compare(start, 4, "true") == 0)
        {
            return true;
        }

        if (content.compare(start, 5, "false") == 0)
        {
            return false;
        }

        return fallback;
    }

    std::vector<std::string> extractObjectsFromArray(
        const std::string& content,
        const std::string& key
    )
    {
        std::vector<std::string> objects;
        std::string pattern = "\"" + key + "\"";
        std::size_t keyPosition = content.find(pattern);

        if (keyPosition == std::string::npos)
        {
            return objects;
        }

        std::size_t arrayStart = content.find('[', keyPosition);

        if (arrayStart == std::string::npos)
        {
            return objects;
        }

        int depth = 0;
        bool insideObject = false;
        std::size_t objectStart = std::string::npos;

        for (std::size_t i = arrayStart; i < content.size(); ++i)
        {
            if (content[i] == ']')
            {
                break;
            }

            if (content[i] == '{')
            {
                if (depth == 0)
                {
                    objectStart = i;
                    insideObject = true;
                }

                depth++;
            }
            else if (content[i] == '}')
            {
                depth--;

                if (insideObject && depth == 0 && objectStart != std::string::npos)
                {
                    objects.push_back(content.substr(objectStart, i - objectStart + 1));
                    insideObject = false;
                    objectStart = std::string::npos;
                }
            }
        }

        return objects;
    }

    Weapon weaponFromName(const std::string& name)
    {
        std::string normalized = normalizeText(name);

        if (normalized == normalizeText("Mains nues")) return WeaponCatalog::createBareHands();
        if (normalized == normalizeText("Épée rouillée")) return WeaponCatalog::createRustySword();
        if (normalized == normalizeText("Lame d'arène")) return WeaponCatalog::createArenaBlade();

        return WeaponCatalog::createRustySword();
    }

    Armor armorFromName(const std::string& name)
    {
        std::string normalized = normalizeText(name);

        if (normalized == normalizeText("Tenue simple")) return ArmorCatalog::createSimpleOutfit();
        if (normalized == normalizeText("Armure en cuir usée")) return ArmorCatalog::createWornLeatherArmor();
        if (normalized == normalizeText("Cotte de maille d'arène")) return ArmorCatalog::createArenaChainmail();

        return ArmorCatalog::createSimpleOutfit();
    }

    Consumable consumableFromName(const std::string& name)
    {
        std::string normalized = normalizeText(name);

        if (normalized == normalizeText("Potion de soin")) return ConsumableCatalog::createBasicHealingPotion();
        if (normalized == normalizeText("Potion de rage")) return ConsumableCatalog::createBasicDamagePotion();
        if (normalized == normalizeText("Potion de soin renforcée")) return ConsumableCatalog::createReinforcedHealingPotion();
        if (normalized == normalizeText("Potion de rage supérieure")) return ConsumableCatalog::createReinforcedDamagePotion();

        return ConsumableCatalog::createBasicHealingPotion();
    }

    void applySavedWeaponDurability(Weapon& weapon, int savedDurability)
    {
        if (weapon.isIndestructible())
        {
            return;
        }

        int loss = weapon.getMaxDurability() - savedDurability;

        if (loss > 0)
        {
            weapon.loseDurability(loss);
        }
    }

    void applySavedArmorDurability(Armor& armor, int savedDurability)
    {
        if (armor.isIndestructible())
        {
            return;
        }

        int loss = armor.getMaxDurability() - savedDurability;

        if (loss > 0)
        {
            armor.loseDurability(loss);
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
    file << "    \"unspentAttributePoints\": " << player.getUnspentAttributePoints() << ",\n";
    file << "    \"attributes\": {\n";
    file << "      \"strength\": " << player.getAttributes().getStrength() << ",\n";
    file << "      \"dexterity\": " << player.getAttributes().getDexterity() << ",\n";
    file << "      \"constitution\": " << player.getAttributes().getConstitution() << ",\n";
    file << "      \"intelligence\": " << player.getAttributes().getIntelligence() << ",\n";
    file << "      \"wisdom\": " << player.getAttributes().getWisdom() << ",\n";
    file << "      \"charisma\": " << player.getAttributes().getCharisma() << "\n";
    file << "    },\n";
    file << "    \"equippedWeaponIndex\": " << player.getEquippedWeaponIndex() << ",\n";
    file << "    \"equippedWeaponName\": \"" << escapeJson(weapon.getName()) << "\",\n";
    file << "    \"equippedArmorIndex\": " << player.getEquippedArmorIndex() << ",\n";
    file << "    \"equippedArmorName\": \"" << escapeJson(armor.getName()) << "\"\n";
    file << "  },\n";

    file << "  \"characterStatistics\": {\n";
    file << "    \"combatsStarted\": " << player.getCombatsStarted() << ",\n";
    file << "    \"victories\": " << player.getVictories() << ",\n";
    file << "    \"defeats\": " << player.getDefeats() << ",\n";
    file << "    \"escapes\": " << player.getEscapes() << ",\n";
    file << "    \"deaths\": " << player.getDeaths() << ",\n";
    file << "    \"enemiesKilled\": " << player.getEnemiesKilled() << ",\n";
    file << "    \"bossesKilled\": " << player.getBossesKilled() << "\n";
    file << "  },\n";

    file << "  \"cheatState\": {\n";
    file << "    \"altered\": " << (player.isAlteredByCheats() ? "true" : "false") << ",\n";
    file << "    \"godMode\": " << (player.isGodModeEnabled() ? "true" : "false") << ",\n";
    file << "    \"infiniteConsumables\": " << (player.hasInfiniteConsumables() ? "true" : "false") << ",\n";
    file << "    \"indestructibleEquipment\": " << (player.hasIndestructibleEquipment() ? "true" : "false") << ",\n";
    file << "    \"equipmentProtection\": " << (player.hasEquipmentProtection() ? "true" : "false") << ",\n";
    file << "    \"storySkip\": " << (player.hasStorySkip() ? "true" : "false") << ",\n";
    file << "    \"godModeKnown\": " << (player.isGodModeKnown() ? "true" : "false") << ",\n";
    file << "    \"infiniteConsumablesKnown\": " << (player.isInfiniteConsumablesKnown() ? "true" : "false") << ",\n";
    file << "    \"indestructibleEquipmentKnown\": " << (player.isIndestructibleEquipmentKnown() ? "true" : "false") << ",\n";
    file << "    \"equipmentProtectionKnown\": " << (player.isEquipmentProtectionKnown() ? "true" : "false") << ",\n";
    file << "    \"storySkipKnown\": " << (player.isStorySkipKnown() ? "true" : "false") << ",\n";
    file << "    \"creatorMessageKnown\": " << (player.isCreatorMessageKnown() ? "true" : "false") << ",\n";
    file << "    \"goldCheatUseCount\": " << player.getGoldCheatUseCount() << ",\n";
    file << "    \"levelCheatUseCount\": " << player.getLevelCheatUseCount() << ",\n";
    file << "    \"maxLevelCheatUseCount\": " << player.getMaxLevelCheatUseCount() << ",\n";
    file << "    \"refundCheatUseCount\": " << player.getRefundCheatUseCount() << ",\n";
    file << "    \"resetCheatUseCount\": " << player.getResetCheatUseCount() << ",\n";
    file << "    \"switchClassCheatUseCount\": " << player.getSwitchClassCheatUseCount() << ",\n";
    file << "    \"refundUsesRemaining\": " << player.getRefundUsesRemaining() << "\n";
    file << "  },\n";

    file << "  \"inventorySnapshot\": {\n";
    file << "    \"note\": \"Temporary readable snapshot. Full loading will be connected later.\",\n";
    file << "    \"weapons\": [\n";

    const std::vector<Weapon>& weapons = player.getInventory().getWeapons();

    for (std::size_t i = 0; i < weapons.size(); i++)
    {
        file << "      {\"name\": \"" << escapeJson(weapons[i].getName()) << "\", \"durability\": "
             << weapons[i].getDurability() << ", \"maxDurability\": " << weapons[i].getMaxDurability() << "}";

        if (i + 1 < weapons.size())
        {
            file << ",";
        }

        file << "\n";
    }

    file << "    ],\n";
    file << "    \"armors\": [\n";

    const std::vector<Armor>& armors = player.getInventory().getArmors();

    for (std::size_t i = 0; i < armors.size(); i++)
    {
        file << "      {\"name\": \"" << escapeJson(armors[i].getName()) << "\", \"durability\": "
             << armors[i].getDurability() << ", \"maxDurability\": " << armors[i].getMaxDurability() << "}";

        if (i + 1 < armors.size())
        {
            file << ",";
        }

        file << "\n";
    }

    file << "    ],\n";
    file << "    \"consumables\": [\n";

    const std::vector<Consumable>& consumables = player.getInventory().getConsumables();

    for (std::size_t i = 0; i < consumables.size(); i++)
    {
        file << "      {\"name\": \"" << escapeJson(consumables[i].getName()) << "\", \"power\": "
             << consumables[i].getPower() << "}";

        if (i + 1 < consumables.size())
        {
            file << ",";
        }

        file << "\n";
    }

    file << "    ],\n";
    file << "    \"materials\": [\n";

    const std::vector<Material>& materials = player.getInventory().getMaterials();

    for (std::size_t i = 0; i < materials.size(); i++)
    {
        file << "      {\"id\": \"" << escapeJson(materials[i].getId()) << "\", \"name\": \""
             << escapeJson(materials[i].getName()) << "\", \"category\": \""
             << escapeJson(materials[i].getCategory()) << "\", \"quantity\": "
             << materials[i].getQuantity() << "}";

        if (i + 1 < materials.size())
        {
            file << ",";
        }

        file << "\n";
    }

    file << "    ]\n";
    file << "  },\n";

    file << "  \"bestiaryRuntimeSnapshot\": [\n";

    std::vector<BestiaryRuntimeRecord> bestiaryRecords = BestiaryRuntimeProgress::getRecords();

    for (std::size_t i = 0; i < bestiaryRecords.size(); i++)
    {
        const BestiaryRuntimeRecord& record = bestiaryRecords[i];

        file << "    {\"category\": \"" << escapeJson(record.category)
             << "\", \"name\": \"" << escapeJson(record.name)
             << "\", \"description\": \"" << escapeJson(record.description)
             << "\", \"status\": \"" << escapeJson(record.status)
             << "\", \"encounters\": " << record.encounters
             << ", \"kills\": " << record.kills
             << ", \"informationBought\": " << (record.informationBought ? "true" : "false")
             << "}";

        if (i + 1 < bestiaryRecords.size())
        {
            file << ",";
        }

        file << "\n";
    }

    file << "  ],\n";
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
    int unspentAttributePoints = extractIntValue(content, "unspentAttributePoints", 0);
    int strength = extractIntValue(content, "strength", 10);
    int dexterity = extractIntValue(content, "dexterity", 10);
    int constitution = extractIntValue(content, "constitution", 10);
    int intelligence = extractIntValue(content, "intelligence", 10);
    int wisdom = extractIntValue(content, "wisdom", 10);
    int charisma = extractIntValue(content, "charisma", 10);
    int equippedWeaponIndex = extractIntValue(content, "equippedWeaponIndex", -1);
    int equippedArmorIndex = extractIntValue(content, "equippedArmorIndex", -1);
    int combatsStarted = extractIntValue(content, "combatsStarted", 0);
    int victories = extractIntValue(content, "victories", 0);
    int defeats = extractIntValue(content, "defeats", 0);
    int escapes = extractIntValue(content, "escapes", 0);
    int deaths = extractIntValue(content, "deaths", 0);
    int enemiesKilled = extractIntValue(content, "enemiesKilled", 0);
    int bossesKilled = extractIntValue(content, "bossesKilled", 0);

    difficulty = difficultyFromText(extractStringValue(content, "difficulty", "Normal"));

    PlayerClass loadedClass = ClassCatalog::createClassByName(className);
    player = Player(characterName, loadedClass);
    player.setRace(raceFromText(raceName));
    player.initializeStarterInventory(difficulty);

    std::vector<std::string> weaponObjects = extractObjectsFromArray(content, "weapons");
    std::vector<std::string> armorObjects = extractObjectsFromArray(content, "armors");
    std::vector<std::string> consumableObjects = extractObjectsFromArray(content, "consumables");
    std::vector<std::string> materialObjects = extractObjectsFromArray(content, "materials");
    std::vector<std::string> bestiaryObjects = extractObjectsFromArray(content, "bestiaryRuntimeSnapshot");

    if (!weaponObjects.empty() || !armorObjects.empty() || !consumableObjects.empty() || !materialObjects.empty())
    {
        player.unequipWeapon();
        player.unequipArmor();
        player.getInventory().clearAll();

        for (const std::string& object : weaponObjects)
        {
            Weapon weapon = weaponFromName(extractStringValue(object, "name", "Épée rouillée"));
            applySavedWeaponDurability(weapon, extractIntValue(object, "durability", weapon.getDurability()));
            player.getInventory().addWeapon(weapon);
        }

        for (const std::string& object : armorObjects)
        {
            Armor armor = armorFromName(extractStringValue(object, "name", "Tenue simple"));
            applySavedArmorDurability(armor, extractIntValue(object, "durability", armor.getDurability()));
            player.getInventory().addArmor(armor);
        }

        for (const std::string& object : consumableObjects)
        {
            player.getInventory().addConsumable(
                consumableFromName(extractStringValue(object, "name", "Potion de soin"))
            );
        }

        for (const std::string& object : materialObjects)
        {
            std::string id = extractStringValue(object, "id", "unknown_material");
            int quantity = extractIntValue(object, "quantity", 1);
            player.getInventory().addMaterial(MaterialCatalog::createById(id, quantity));
        }
    }

    player.getInventory().setGold(gold);
    player.setLoadedProgress(level, experience, hp);

    DndAttributes loadedAttributes;
    loadedAttributes.setValues(strength, dexterity, constitution, intelligence, wisdom, charisma);
    player.setLoadedAttributes(loadedAttributes, unspentAttributePoints);
    player.setLoadedStatistics(
        combatsStarted,
        victories,
        defeats,
        escapes,
        deaths,
        enemiesKilled,
        bossesKilled
    );

    if (equippedWeaponIndex >= 0)
    {
        player.equipWeapon(equippedWeaponIndex);
    }

    if (equippedArmorIndex >= 0)
    {
        player.equipArmor(equippedArmorIndex);
    }

    bool altered = extractBoolValue(content, "altered", false);
    bool godMode = extractBoolValue(content, "godMode", false);
    bool infiniteConsumables = extractBoolValue(content, "infiniteConsumables", false);
    bool indestructibleEquipment = extractBoolValue(content, "indestructibleEquipment", false);
    bool equipmentProtection = extractBoolValue(content, "equipmentProtection", false);
    bool storySkip = extractBoolValue(content, "storySkip", false);

    player.setCheatState(
        altered,
        godMode,
        infiniteConsumables,
        indestructibleEquipment,
        equipmentProtection,
        storySkip,
        extractIntValue(content, "refundUsesRemaining", 3)
    );

    if (extractBoolValue(content, "godModeKnown", godMode) && !godMode)
    {
        player.toggleGodMode();
        player.toggleGodMode();
    }

    if (extractBoolValue(content, "infiniteConsumablesKnown", infiniteConsumables) && !infiniteConsumables)
    {
        player.toggleInfiniteConsumables();
        player.toggleInfiniteConsumables();
    }

    if (extractBoolValue(content, "indestructibleEquipmentKnown", indestructibleEquipment) && !indestructibleEquipment)
    {
        player.toggleIndestructibleEquipment();
        player.toggleIndestructibleEquipment();
    }

    if (extractBoolValue(content, "equipmentProtectionKnown", equipmentProtection) && !equipmentProtection)
    {
        player.toggleEquipmentProtection();
        player.toggleEquipmentProtection();
    }

    if (extractBoolValue(content, "storySkipKnown", storySkip) && !storySkip)
    {
        player.toggleStorySkip();
        player.toggleStorySkip();
    }

    if (extractBoolValue(content, "creatorMessageKnown", false))
    {
        player.markCreatorMessageSeen();
    }

    int goldUses = extractIntValue(content, "goldCheatUseCount", 0);
    for (int i = 0; i < goldUses; i++)
    {
        player.recordGoldCheatUse();
    }

    int levelUses = extractIntValue(content, "levelCheatUseCount", 0);
    for (int i = 0; i < levelUses; i++)
    {
        player.recordLevelCheatUse();
    }

    int maxLevelUses = extractIntValue(content, "maxLevelCheatUseCount", 0);
    for (int i = 0; i < maxLevelUses; i++)
    {
        player.recordMaxLevelCheatUse();
    }

    int refundUses = extractIntValue(content, "refundCheatUseCount", 0);
    for (int i = 0; i < refundUses; i++)
    {
        player.recordRefundCheatUse();
    }

    int resetUses = extractIntValue(content, "resetCheatUseCount", 0);
    for (int i = 0; i < resetUses; i++)
    {
        player.recordResetCheatUse();
    }

    int switchUses = extractIntValue(content, "switchClassCheatUseCount", 0);
    for (int i = 0; i < switchUses; i++)
    {
        player.recordSwitchClassCheatUse();
    }

    if (!altered && !player.isAlteredByCheats())
    {
        player.setCheatState(false, false, false, false, false, false, player.getRefundUsesRemaining());
    }

    BestiaryRuntimeProgress::clear();

    for (const std::string& object : bestiaryObjects)
    {
        BestiaryRuntimeRecord record;
        record.category = extractStringValue(object, "category", "Entités hostiles / ennemis");
        record.name = extractStringValue(object, "name", "");
        record.description = extractStringValue(object, "description", "Entrée sauvegardée.");
        record.status = extractStringValue(object, "status", "Rencontré");
        record.encounters = extractIntValue(object, "encounters", 0);
        record.kills = extractIntValue(object, "kills", 0);
        record.informationBought = extractBoolValue(object, "informationBought", false);

        BestiaryRuntimeProgress::importRecord(record);
    }

    return true;
}



bool SaveManager::movePlayableCharacterToDead(
    const std::string& accountName,
    const std::string& characterName
)
{
    if (!ensureSaveDirectories())
    {
        return false;
    }

    std::string sourcePath = getCharacterSavePath(accountName, characterName);
    std::string destinationPath = SAVE_ROOT
        + "/characters/dead/"
        + buildSafeFileName(accountName)
        + "__"
        + buildSafeFileName(characterName)
        + "__dead.json";

    try
    {
        if (!std::filesystem::exists(sourcePath))
        {
            return false;
        }

        std::filesystem::copy_file(
            sourcePath,
            destinationPath,
            std::filesystem::copy_options::overwrite_existing
        );

        std::filesystem::remove(sourcePath);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool SaveManager::deletePlayableCharacter(const CharacterSaveSummary& summary)
{
    if (!ensureSaveDirectories())
    {
        return false;
    }

    try
    {
        if (summary.path.empty() || !std::filesystem::exists(summary.path))
        {
            return false;
        }

        return std::filesystem::remove(summary.path);
    }
    catch (...)
    {
        return false;
    }
}

bool SaveManager::deleteAccountAndLinkedCharacters(const std::string& accountName)
{
    if (!ensureSaveDirectories())
    {
        return false;
    }

    bool success = true;
    std::string safeAccount = buildSafeFileName(accountName);
    std::string accountPrefix = safeAccount + "__";

    std::vector<std::string> foldersToClean = {
        SAVE_ROOT + "/characters/playable",
        SAVE_ROOT + "/characters/dead",
        SAVE_ROOT + "/bestiary",
        SAVE_ROOT + "/materials"
    };

    try
    {
        for (const std::string& folder : foldersToClean)
        {
            if (!std::filesystem::exists(folder))
            {
                continue;
            }

            for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(folder))
            {
                if (!entry.is_regular_file())
                {
                    continue;
                }

                std::string fileName = entry.path().filename().string();

                if (fileName.rfind(accountPrefix, 0) == 0 || fileName.rfind(safeAccount + ".", 0) == 0)
                {
                    if (!std::filesystem::remove(entry.path()))
                    {
                        success = false;
                    }
                }
            }
        }

        std::string accountPath = getAccountSavePath(accountName);

        if (std::filesystem::exists(accountPath))
        {
            if (!std::filesystem::remove(accountPath))
            {
                success = false;
            }
        }

        return success;
    }
    catch (...)
    {
        return false;
    }
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
