// EN: SaveManager.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SaveManager.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file prepares Dinotofu save helpers. Code identifiers stay in English; player-facing text can stay in French.
// Français : Ce fichier prépare les helpers de sauvegarde Dinotofu. Les identifiants restent en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Minimal JSON-like save manager for accounts, playable characters, future bestiary and materials.

#include "save/SaveManager.hpp"

#include "character/CharacterRace.hpp"
#include "class_system/ClassCatalog.hpp"
#include "item/armor/ArmorCatalog.hpp"
#include "item/consumable/ConsumableCatalog.hpp"
#include "item/consumable/Consumable.hpp"
#include "item/consumable/ConsumableType.hpp"
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

    // EN: writeFileContent declares or implements a focused behavior used by this module.
    // FR: writeFileContent déclare ou implémente un comportement précis utilisé par ce module.
    bool writeFileContent(const std::string& path, const std::string& content)
    {
        std::ofstream file(path);

        if (!file.is_open())
        {
            return false;
        }

        file << content;
        return true;
    }

    std::string withJsonBooleanField(
        const std::string& content,
        const std::string& key,
        bool value
    )
    {
        std::string pattern = "\"" + key + "\"";
        std::size_t keyPosition = content.find(pattern);
        std::string replacement = "\"" + key + "\": " + std::string(value ? "true" : "false");

        if (keyPosition != std::string::npos)
        {
            std::size_t colonPosition = content.find(':', keyPosition);
            if (colonPosition == std::string::npos)
            {
                return content;
            }

            std::size_t valueStart = content.find_first_not_of(" \t\r\n", colonPosition + 1);
            if (valueStart == std::string::npos)
            {
                return content;
            }

            std::size_t valueEnd = valueStart;
            while (valueEnd < content.size()
                   && content[valueEnd] != ','
                   && content[valueEnd] != '\n'
                   && content[valueEnd] != '}')
            {
                ++valueEnd;
            }

            std::string updated = content;
            updated.replace(keyPosition, valueEnd - keyPosition, replacement);
            return updated;
        }

        std::string characterPattern = "\"character\"";
        std::size_t characterPosition = content.find(characterPattern);
        std::size_t openingBrace = content.find('{', characterPosition);

        if (characterPosition == std::string::npos || openingBrace == std::string::npos)
        {
            return content;
        }

        std::string updated = content;
        updated.insert(openingBrace + 1, "\n    " + replacement + ",");
        return updated;
    }

    // EN: createBackupIfSaveExists declares or implements a focused behavior used by this module.
    // FR: createBackupIfSaveExists déclare ou implémente un comportement précis utilisé par ce module.
    void createBackupIfSaveExists(const std::string& path)
    {
        try
        {
            if (std::filesystem::exists(path))
            {
                std::filesystem::copy_file(
                    path,
                    path + ".bak",
                    std::filesystem::copy_options::overwrite_existing
                );
            }
        }
        catch (...)
        {
            // Backup failure must not block the current save.
        }
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

    std::string weaponTypeToSaveText(WeaponType type)
    {
        switch (type)
        {
            case WeaponType::Sword: return "Sword";
            case WeaponType::Dagger: return "Dagger";
            case WeaponType::Axe: return "Axe";
            case WeaponType::Hammer: return "Hammer";
            case WeaponType::Spear: return "Spear";
            case WeaponType::Staff: return "Staff";
            case WeaponType::Bow: return "Bow";
            case WeaponType::BareHands: return "BareHands";
            default: return "Unknown";
        }
    }

    // EN: weaponTypeFromSaveText declares or implements a focused behavior used by this module.
    // FR: weaponTypeFromSaveText déclare ou implémente un comportement précis utilisé par ce module.
    WeaponType weaponTypeFromSaveText(const std::string& value)
    {
        std::string normalized = normalizeText(value);

        if (normalized == normalizeText("Sword") || normalized == normalizeText("Épée")) return WeaponType::Sword;
        if (normalized == normalizeText("Dagger") || normalized == normalizeText("Dague")) return WeaponType::Dagger;
        if (normalized == normalizeText("Axe") || normalized == normalizeText("Hache")) return WeaponType::Axe;
        if (normalized == normalizeText("Hammer") || normalized == normalizeText("Marteau")) return WeaponType::Hammer;
        if (normalized == normalizeText("Spear") || normalized == normalizeText("Lance")) return WeaponType::Spear;
        if (normalized == normalizeText("Staff") || normalized == normalizeText("Bâton")) return WeaponType::Staff;
        if (normalized == normalizeText("Bow") || normalized == normalizeText("Arc")) return WeaponType::Bow;
        if (normalized == normalizeText("BareHands") || normalized == normalizeText("Mains nues")) return WeaponType::BareHands;

        return WeaponType::Sword;
    }

    std::string armorTypeToSaveText(ArmorType type)
    {
        switch (type)
        {
            case ArmorType::Cloth: return "Cloth";
            case ArmorType::Leather: return "Leather";
            case ArmorType::Chainmail: return "Chainmail";
            case ArmorType::Plate: return "Plate";
            case ArmorType::Magical: return "Magical";
            default: return "Unknown";
        }
    }

    // EN: armorTypeFromSaveText declares or implements a focused behavior used by this module.
    // FR: armorTypeFromSaveText déclare ou implémente un comportement précis utilisé par ce module.
    ArmorType armorTypeFromSaveText(const std::string& value)
    {
        std::string normalized = normalizeText(value);

        if (normalized == normalizeText("Cloth") || normalized == normalizeText("Tissu")) return ArmorType::Cloth;
        if (normalized == normalizeText("Leather") || normalized == normalizeText("Cuir")) return ArmorType::Leather;
        if (normalized == normalizeText("Chainmail") || normalized == normalizeText("Maille")) return ArmorType::Chainmail;
        if (normalized == normalizeText("Plate") || normalized == normalizeText("Plaque")) return ArmorType::Plate;
        if (normalized == normalizeText("Magical") || normalized == normalizeText("Magique")) return ArmorType::Magical;

        return ArmorType::Unknown;
    }

    // EN: weaponFromName declares or implements a focused behavior used by this module.
    // FR: weaponFromName déclare ou implémente un comportement précis utilisé par ce module.
    Weapon weaponFromName(const std::string& name)
    {
        std::string normalized = normalizeText(name);

        if (normalized == normalizeText("Mains nues")) return WeaponCatalog::createBareHands();
        if (normalized == normalizeText("Épée rouillée")) return WeaponCatalog::createRustySword();
        if (normalized == normalizeText("Lame d'arène")) return WeaponCatalog::createArenaBlade();
        if (normalized.find(normalizeText("Lame de récupération")) != std::string::npos)
        {
            bool special = normalized.find(normalizeText("particularité")) != std::string::npos;
            Weapon base(
                "Lame de récupération",
                "Une lame assemblée avec des matériaux récupérés. Pas noble, mais plus fiable qu'une rouille abandonnée.",
                65,
                WeaponType::Sword,
                4,
                8,
                5,
                115
            );

            if (!special) return base;

            return Weapon(
                base.getName() + " — particularité précise",
                base.getDescription() + " Une faible particularité née de matériaux exceptionnels rend l'équilibre légèrement meilleur.",
                base.getValue() + 35,
                base.getType(),
                base.getMinDamageBonus() + 1,
                base.getMaxDamageBonus() + 2,
                base.getCriticalBonus() + 1,
                base.getMaxDurability() + 8
            );
        }

        return WeaponCatalog::createRustySword();
    }

    // EN: armorFromName declares or implements a focused behavior used by this module.
    // FR: armorFromName déclare ou implémente un comportement précis utilisé par ce module.
    Armor armorFromName(const std::string& name)
    {
        std::string normalized = normalizeText(name);

        if (normalized == normalizeText("Tenue simple")) return ArmorCatalog::createSimpleOutfit();
        if (normalized == normalizeText("Armure en cuir usée")) return ArmorCatalog::createWornLeatherArmor();
        if (normalized == normalizeText("Cotte de maille d'arène")) return ArmorCatalog::createArenaChainmail();
        if (normalized.find(normalizeText("Armure de chasseur rafistolée")) != std::string::npos)
        {
            bool special = normalized.find(normalizeText("particularité")) != std::string::npos;
            Armor base(
                "Armure de chasseur rafistolée",
                "Une armure souple faite pour survivre aux mauvaises rencontres plutôt que briller dans une forge royale.",
                90,
                ArmorType::Leather,
                38,
                4,
                125
            );

            if (!special) return base;

            return Armor(
                base.getName() + " — particularité renforcée",
                base.getDescription() + " Une faible particularité née de matériaux exceptionnels stabilise mieux les protections.",
                base.getValue() + 40,
                base.getType(),
                base.getMaxHpBonus() + 6,
                base.getDamageReduction() + 1,
                base.getMaxDurability() + 10
            );
        }

        return ArmorCatalog::createSimpleOutfit();
    }

    // EN: consumableFromName declares or implements a focused behavior used by this module.
    // FR: consumableFromName déclare ou implémente un comportement précis utilisé par ce module.
    Consumable consumableFromName(const std::string& name)
    {
        std::string normalized = normalizeText(name);

        if (normalized == normalizeText("Potion de soin")) return ConsumableCatalog::createBasicHealingPotion();
        if (normalized == normalizeText("Potion de rage")) return ConsumableCatalog::createBasicDamagePotion();
        if (normalized == normalizeText("Potion de soin renforcée")) return ConsumableCatalog::createReinforcedHealingPotion();
        if (normalized == normalizeText("Potion de rage supérieure")) return ConsumableCatalog::createReinforcedDamagePotion();

        if (normalized.find(normalizeText("particularité stable")) != std::string::npos)
        {
            bool reinforced = normalized.find(normalizeText("renforcée")) != std::string::npos;
            Consumable base = reinforced
                ? ConsumableCatalog::createReinforcedHealingPotion()
                : ConsumableCatalog::createBasicHealingPotion();
            return Consumable(
                base.getName() + " — particularité stable",
                base.getDescription() + " Une faible particularité issue de matériaux exceptionnels augmente légèrement son effet.",
                base.getValue() + 18,
                ConsumableType::Healing,
                base.getPower() + 8
            );
        }

        if (normalized.find(normalizeText("particularité agressive")) != std::string::npos)
        {
            bool superior = normalized.find(normalizeText("supérieure")) != std::string::npos;
            Consumable base = superior
                ? ConsumableCatalog::createReinforcedDamagePotion()
                : ConsumableCatalog::createBasicDamagePotion();
            return Consumable(
                base.getName() + " — particularité agressive",
                base.getDescription() + " Une faible particularité issue de matériaux exceptionnels augmente légèrement son effet.",
                base.getValue() + 20,
                ConsumableType::Damage,
                base.getPower() + 6
            );
        }

        return ConsumableCatalog::createBasicHealingPotion();
    }

    // EN: applySavedWeaponDurability declares or implements a focused behavior used by this module.
    // FR: applySavedWeaponDurability déclare ou implémente un comportement précis utilisé par ce module.
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

    // EN: applySavedArmorDurability declares or implements a focused behavior used by this module.
    // FR: applySavedArmorDurability déclare ou implémente un comportement précis utilisé par ce module.
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

    // EN: difficultyFromText declares or implements a focused behavior used by this module.
    // FR: difficultyFromText déclare ou implémente un comportement précis utilisé par ce module.
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

    // EN: raceFromText declares or implements a focused behavior used by this module.
    // FR: raceFromText déclare ou implémente un comportement précis utilisé par ce module.
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

    // EN: parseCharacterSummary declares or implements a focused behavior used by this module.
    // FR: parseCharacterSummary déclare ou implémente un comportement précis utilisé par ce module.
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
        summary.clone = extractBoolValue(content, "clone", false);

        return summary;
    }
}

// EN: ensureSaveDirectories declares or implements a focused behavior used by this module.
// FR: ensureSaveDirectories déclare ou implémente un comportement précis utilisé par ce module.
bool SaveManager::ensureSaveDirectories()
{
    try
    {
        std::filesystem::create_directories(SAVE_ROOT + "/accounts");
        std::filesystem::create_directories(SAVE_ROOT + "/characters/playable");
        std::filesystem::create_directories(SAVE_ROOT + "/characters/dead");
        std::filesystem::create_directories(SAVE_ROOT + "/bestiary");
        std::filesystem::create_directories(SAVE_ROOT + "/materials");
        std::filesystem::create_directories(SAVE_ROOT + "/exported_accounts");
        std::filesystem::create_directories(SAVE_ROOT + "/import_accounts");
        return true;
    }
    catch (...)
    {
        return false;
    }
}

// EN: accountExists declares or implements a focused behavior used by this module.
// FR: accountExists déclare ou implémente un comportement précis utilisé par ce module.
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

    std::string savePath = getAccountSavePath(accountName);
    createBackupIfSaveExists(savePath);

    std::ofstream file(savePath);

    if (!file.is_open())
    {
        return false;
    }

    file << "{\n";
    file << "  \"saveVersion\": 13,\n";
    file << "  \"backupPolicy\": \"previous_save_written_to_bak_when_possible\",\n";
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

    std::string savePath = getCharacterSavePath(accountName, player.getName());
    createBackupIfSaveExists(savePath);

    std::ofstream file(savePath);

    if (!file.is_open())
    {
        return false;
    }

    Weapon weapon = player.getEquippedWeapon();
    Armor armor = player.getEquippedArmor();

    file << "{\n";
    file << "  \"saveVersion\": 13,\n";
    file << "  \"backupPolicy\": \"previous_save_written_to_bak_when_possible\",\n";
    file << "  \"account\": \"" << escapeJson(accountName) << "\",\n";
    file << "  \"difficulty\": \"" << escapeJson(difficultyToText(difficulty)) << "\",\n";
    file << "  \"character\": {\n";
    file << "    \"name\": \"" << escapeJson(player.getName()) << "\",\n";
    file << "    \"race\": \"" << escapeJson(player.getRaceText()) << "\",\n";
    file << "    \"class\": \"" << escapeJson(player.getType()) << "\",\n";
    file << "    \"clone\": " << (player.isClone() ? "true" : "false") << ",\n";
    file << "    \"level\": " << player.getLevel() << ",\n";
    file << "    \"experience\": " << player.getExperience() << ",\n";
    file << "    \"hp\": " << player.getHp() << ",\n";
    file << "    \"maxHp\": " << player.getMaxHp() << ",\n";
    file << "    \"minDamage\": " << player.getMinDamage() << ",\n";
    file << "    \"maxDamage\": " << player.getMaxDamage() << ",\n";
    file << "    \"criticalDamage\": " << player.getCriticalDamage() << ",\n";
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
    file << "    \"bossesKilled\": " << player.getBossesKilled() << ",\n";
    file << "    \"pvpVictories\": " << player.getPvpVictories() << ",\n";
    file << "    \"pvpDefeats\": " << player.getPvpDefeats() << "\n";
    file << "  },\n";

    file << "  \"pvpLethalEliminations\": [\n";
    const std::vector<std::string>& lethalEliminations = player.getPvpLethalEliminations();

    for (std::size_t i = 0; i < lethalEliminations.size(); i++)
    {
        file << "    {\"label\": \"" << escapeJson(lethalEliminations[i]) << "\"}";

        if (i + 1 < lethalEliminations.size())
        {
            file << ",";
        }

        file << "\n";
    }

    file << "  ],\n";

    file << "  \"recentCombatEquipmentUsage\": [\n";
    const std::vector<std::string>& recentEquipmentUsage = player.getRecentCombatEquipmentUsage();

    for (std::size_t i = 0; i < recentEquipmentUsage.size(); i++)
    {
        file << "    {\"label\": \"" << escapeJson(recentEquipmentUsage[i]) << "\"}";

        if (i + 1 < recentEquipmentUsage.size())
        {
            file << ",";
        }

        file << "\n";
    }

    file << "  ],\n";

    file << "  \"skillProgressSnapshot\": {\n";
    file << "    \"unlockedPassiveSkills\": [";
    const std::vector<std::string>& passiveSkills = player.getUnlockedPassiveSkills();
    for (std::size_t i = 0; i < passiveSkills.size(); ++i)
    {
        file << "{\"id\": \"" << escapeJson(passiveSkills[i]) << "\"}";
        if (i + 1 < passiveSkills.size()) file << ", ";
    }
    file << "],\n";
    file << "    \"unlockedActiveSkills\": [";
    const std::vector<std::string>& activeSkills = player.getUnlockedActiveSkills();
    for (std::size_t i = 0; i < activeSkills.size(); ++i)
    {
        file << "{\"id\": \"" << escapeJson(activeSkills[i]) << "\"}";
        if (i + 1 < activeSkills.size()) file << ", ";
    }
    file << "],\n";
    file << "    \"daggerKillProgress\": " << player.getDaggerKillProgress() << ",\n";
    file << "    \"bowKillProgress\": " << player.getBowKillProgress() << ",\n";
    file << "    \"bareHandKillProgress\": " << player.getBareHandKillProgress() << ",\n";
    file << "    \"staffKillProgress\": " << player.getStaffKillProgress() << "\n";
    file << "  },\n";

    file << "  \"bossRegistry\": {\n";
    file << "    \"unlockedBossIds\": [";
    const std::vector<int>& unlockedBossIds = player.getUnlockedBossIds();
    for (std::size_t i = 0; i < unlockedBossIds.size(); ++i)
    {
        file << "{\"id\":" << unlockedBossIds[i] << "}";
        if (i + 1 < unlockedBossIds.size())
        {
            file << ", ";
        }
    }
    file << "],\n";
    file << "    \"recentBossIds\": [";
    const std::vector<int>& recentBossIds = player.getRecentBossIds();
    for (std::size_t i = 0; i < recentBossIds.size(); ++i)
    {
        file << "{\"id\":" << recentBossIds[i] << "}";
        if (i + 1 < recentBossIds.size())
        {
            file << ", ";
        }
    }
    file << "]\n";
    file << "  },\n";

    file << "  \"cheatState\": {\n";
    file << "    \"altered\": " << (player.isAlteredByCheats() ? "true" : "false") << ",\n";
    file << "    \"godMode\": " << (player.isGodModeEnabled() ? "true" : "false") << ",\n";
    file << "    \"infiniteConsumables\": " << (player.hasInfiniteConsumables() ? "true" : "false") << ",\n";
    file << "    \"indestructibleEquipment\": " << (player.hasIndestructibleEquipment() ? "true" : "false") << ",\n";
    file << "    \"equipmentProtection\": " << (player.hasEquipmentProtection() ? "true" : "false") << ",\n";
    file << "    \"storySkip\": " << (player.hasStorySkip() ? "true" : "false") << ",\n";
    file << "    \"specialChallengeAccess\": " << (player.hasSpecialChallengeAccess() ? "true" : "false") << ",\n";
    file << "    \"godModeKnown\": " << (player.isGodModeKnown() ? "true" : "false") << ",\n";
    file << "    \"infiniteConsumablesKnown\": " << (player.isInfiniteConsumablesKnown() ? "true" : "false") << ",\n";
    file << "    \"indestructibleEquipmentKnown\": " << (player.isIndestructibleEquipmentKnown() ? "true" : "false") << ",\n";
    file << "    \"equipmentProtectionKnown\": " << (player.isEquipmentProtectionKnown() ? "true" : "false") << ",\n";
    file << "    \"storySkipKnown\": " << (player.isStorySkipKnown() ? "true" : "false") << ",\n";
    file << "    \"specialChallengeAccessKnown\": " << (player.isSpecialChallengeAccessKnown() ? "true" : "false") << ",\n";
    file << "    \"creatorMessageKnown\": " << (player.isCreatorMessageKnown() ? "true" : "false") << ",\n";
    file << "    \"goldCheatUseCount\": " << player.getGoldCheatUseCount() << ",\n";
    file << "    \"levelCheatUseCount\": " << player.getLevelCheatUseCount() << ",\n";
    file << "    \"maxLevelCheatUseCount\": " << player.getMaxLevelCheatUseCount() << ",\n";
    file << "    \"refundCheatUseCount\": " << player.getRefundCheatUseCount() << ",\n";
    file << "    \"resetCheatUseCount\": " << player.getResetCheatUseCount() << ",\n";
    file << "    \"switchClassCheatUseCount\": " << player.getSwitchClassCheatUseCount() << ",\n";
    file << "    \"lethalCheatAttemptCount\": " << player.getLethalCheatAttemptCount() << ",\n";
    file << "    \"worldGazeCombatsRemaining\": " << player.getWorldGazeCombatsRemaining() << ",\n";
    file << "    \"worldGazeMaxHpPenalty\": " << player.getWorldGazeMaxHpPenalty() << ",\n";
    file << "    \"refundUsesRemaining\": " << player.getRefundUsesRemaining() << "\n";
    file << "  },\n";

    file << "  \"bossConsequences\": {\n";
    file << "    \"zelefCorrosionPresent\": " << (player.hasZelefCorrosionPresent() ? "true" : "false") << ",\n";
    file << "    \"zelefMaxHpStolen\": " << player.getZelefMaxHpStolen() << ",\n";
    file << "    \"grinkaBossTheftPresent\": " << (player.hasGrinkaBossTheftPresent() ? "true" : "false") << ",\n";
    file << "    \"grinkaStolenGold\": " << player.getGrinkaStolenGold() << ",\n";
    file << "    \"grinkaStolenWeapons\": [";
    if (player.hasGrinkaStolenWeapon())
    {
        Weapon stolenWeapon = player.getGrinkaStolenWeapon();
        file << "{\"name\": \"" << escapeJson(stolenWeapon.getName())
             << "\", \"description\": \"" << escapeJson(stolenWeapon.getDescription())
             << "\", \"value\": " << stolenWeapon.getValue()
             << ", \"type\": \"" << weaponTypeToSaveText(stolenWeapon.getType())
             << "\", \"minDamageBonus\": " << stolenWeapon.getMinDamageBonus()
             << ", \"maxDamageBonus\": " << stolenWeapon.getMaxDamageBonus()
             << ", \"criticalBonus\": " << stolenWeapon.getCriticalBonus()
             << ", \"durability\": " << stolenWeapon.getDurability()
             << ", \"maxDurability\": " << stolenWeapon.getMaxDurability() << "}";
    }
    file << "],\n";
    file << "    \"grinkaStolenArmors\": [";
    if (player.hasGrinkaStolenArmor())
    {
        Armor stolenArmor = player.getGrinkaStolenArmor();
        file << "{\"name\": \"" << escapeJson(stolenArmor.getName())
             << "\", \"description\": \"" << escapeJson(stolenArmor.getDescription())
             << "\", \"value\": " << stolenArmor.getValue()
             << ", \"type\": \"" << armorTypeToSaveText(stolenArmor.getType())
             << "\", \"maxHpBonus\": " << stolenArmor.getMaxHpBonus()
             << ", \"damageReduction\": " << stolenArmor.getDamageReduction()
             << ", \"durability\": " << stolenArmor.getDurability()
             << ", \"maxDurability\": " << stolenArmor.getMaxDurability() << "}";
    }
    file << "]\n";
    file << "  },\n";

    file << "  \"inventorySnapshot\": {\n";
    file << "    \"note\": \"Readable snapshot now reloaded for weapons, armors, consumables and materials.\",\n";
    file << "    \"weapons\": [\n";

    const std::vector<Weapon>& weapons = player.getInventory().getWeapons();

    for (std::size_t i = 0; i < weapons.size(); i++)
    {
        file << "      {\"name\": \"" << escapeJson(weapons[i].getName())
             << "\", \"description\": \"" << escapeJson(weapons[i].getDescription())
             << "\", \"value\": " << weapons[i].getValue()
             << ", \"type\": \"" << weaponTypeToSaveText(weapons[i].getType())
             << "\", \"minDamageBonus\": " << weapons[i].getMinDamageBonus()
             << ", \"maxDamageBonus\": " << weapons[i].getMaxDamageBonus()
             << ", \"criticalBonus\": " << weapons[i].getCriticalBonus()
             << ", \"durability\": " << weapons[i].getDurability()
             << ", \"maxDurability\": " << weapons[i].getMaxDurability() << "}";

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
        file << "      {\"name\": \"" << escapeJson(armors[i].getName())
             << "\", \"description\": \"" << escapeJson(armors[i].getDescription())
             << "\", \"value\": " << armors[i].getValue()
             << ", \"type\": \"" << armorTypeToSaveText(armors[i].getType())
             << "\", \"maxHpBonus\": " << armors[i].getMaxHpBonus()
             << ", \"damageReduction\": " << armors[i].getDamageReduction()
             << ", \"durability\": " << armors[i].getDurability()
             << ", \"maxDurability\": " << armors[i].getMaxDurability() << "}";

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
             << escapeJson(materials[i].getCategory()) << "\", \"quality\": \""
             << escapeJson(materials[i].getQuality()) << "\", \"quantity\": "
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

    file << "  \"questLogSnapshot\": [\n";

    const std::vector<Quest>& quests = player.getQuestLog().getQuests();

    for (std::size_t i = 0; i < quests.size(); i++)
    {
        const Quest& quest = quests[i];

        file << "    {\"id\": \"" << escapeJson(quest.id)
             << "\", \"rank\": \"" << escapeJson(quest.rank)
             << "\", \"title\": \"" << escapeJson(quest.title)
             << "\", \"origin\": \"" << escapeJson(quest.origin)
             << "\", \"client\": \"" << escapeJson(quest.client)
             << "\", \"location\": \"" << escapeJson(quest.location)
             << "\", \"objective\": \"" << escapeJson(quest.objective)
             << "\", \"objectiveType\": \"" << escapeJson(quest.objectiveType)
             << "\", \"targetFamily\": \"" << escapeJson(quest.targetFamily)
             << "\", \"rewardExperience\": " << quest.rewardExperience
             << ", \"rewardGold\": " << quest.rewardGold
             << ", \"requiredMaterialId\": \"" << escapeJson(quest.requiredMaterialId)
             << "\", \"requiredMaterialName\": \"" << escapeJson(quest.requiredMaterialName)
             << "\", \"requiredMaterialQuantity\": " << quest.requiredMaterialQuantity
             << ", \"progress\": " << quest.progress
             << ", \"target\": " << quest.target
             << ", \"guildQuest\": " << (quest.guildQuest ? "true" : "false")
             << ", \"accepted\": " << (quest.accepted ? "true" : "false")
             << ", \"completed\": " << (quest.completed ? "true" : "false")
             << ", \"turnedIn\": " << (quest.turnedIn ? "true" : "false")
             << "}";

        if (i + 1 < quests.size())
        {
            file << ",";
        }

        file << "\n";
    }

    file << "  ],\n";
    file << "  \"futureData\": {\n";
    file << "    \"inventoryFullSerialization\": \"stable_for_current_weapons_armors_consumables_materials_and_repair_kits\",\n";
    file << "    \"playerCombatStats\": \"saved_and_reloaded\",\n";
    file << "    \"bestiary\": \"runtime_records_saved_and_reloaded\",\n";
    file << "    \"materials\": \"saved_and_reloaded_by_id_including_used_repair_kits\",\n";
    file << "    \"cheats\": \"states_and_known_codes_saved_and_reloaded\",\n";
    file << "    \"quests\": \"guild_and_personal_quest_log_saved_and_reloaded\",\n";
    file << "    \"pvpStatistics\": \"victories_defeats_lethal_eliminations_and_clone_status_saved_and_reloaded\",\n";
    file << "    \"skills\": \"passive_active_unlocks_and_weapon_progress_saved_and_reloaded\",\n";
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
    int loadedMaxHp = extractIntValue(content, "maxHp", 100);
    int loadedMinDamage = extractIntValue(content, "minDamage", 1);
    int loadedMaxDamage = extractIntValue(content, "maxDamage", 5);
    int loadedCriticalDamage = extractIntValue(content, "criticalDamage", 10);
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
    int pvpVictories = extractIntValue(content, "pvpVictories", 0);
    int pvpDefeats = extractIntValue(content, "pvpDefeats", 0);

    difficulty = difficultyFromText(extractStringValue(content, "difficulty", "Normal"));

    PlayerClass loadedClass = ClassCatalog::createClassByName(className);
    player = Player(characterName, loadedClass);
    player.setRace(raceFromText(raceName));
    player.initializeStarterInventory(difficulty);
    player.setClone(extractBoolValue(content, "clone", false));

    std::vector<std::string> weaponObjects = extractObjectsFromArray(content, "weapons");
    std::vector<std::string> armorObjects = extractObjectsFromArray(content, "armors");
    std::vector<std::string> consumableObjects = extractObjectsFromArray(content, "consumables");
    std::vector<std::string> materialObjects = extractObjectsFromArray(content, "materials");
    std::vector<std::string> bestiaryObjects = extractObjectsFromArray(content, "bestiaryRuntimeSnapshot");
    std::vector<std::string> questObjects = extractObjectsFromArray(content, "questLogSnapshot");
    std::vector<std::string> lethalEliminationObjects = extractObjectsFromArray(content, "pvpLethalEliminations");
    std::vector<std::string> recentEquipmentUsageObjects = extractObjectsFromArray(content, "recentCombatEquipmentUsage");
    std::vector<std::string> passiveSkillObjects = extractObjectsFromArray(content, "unlockedPassiveSkills");
    std::vector<std::string> activeSkillObjects = extractObjectsFromArray(content, "unlockedActiveSkills");
    std::vector<std::string> unlockedBossObjects = extractObjectsFromArray(content, "unlockedBossIds");
    std::vector<std::string> recentBossObjects = extractObjectsFromArray(content, "recentBossIds");
    std::vector<std::string> grinkaStolenWeaponObjects = extractObjectsFromArray(content, "grinkaStolenWeapons");
    std::vector<std::string> grinkaStolenArmorObjects = extractObjectsFromArray(content, "grinkaStolenArmors");

    if (!weaponObjects.empty() || !armorObjects.empty() || !consumableObjects.empty() || !materialObjects.empty())
    {
        player.unequipWeapon();
        player.unequipArmor();
        player.getInventory().clearAll();

        for (const std::string& object : weaponObjects)
        {
            std::string weaponName = extractStringValue(object, "name", "Épée rouillée");
            Weapon fallback = weaponFromName(weaponName);

            Weapon weapon(
                weaponName,
                extractStringValue(object, "description", fallback.getDescription()),
                extractIntValue(object, "value", fallback.getValue()),
                weaponTypeFromSaveText(extractStringValue(object, "type", weaponTypeToSaveText(fallback.getType()))),
                extractIntValue(object, "minDamageBonus", fallback.getMinDamageBonus()),
                extractIntValue(object, "maxDamageBonus", fallback.getMaxDamageBonus()),
                extractIntValue(object, "criticalBonus", fallback.getCriticalBonus()),
                extractIntValue(object, "maxDurability", fallback.getMaxDurability())
            );

            applySavedWeaponDurability(weapon, extractIntValue(object, "durability", weapon.getDurability()));
            player.getInventory().addWeapon(weapon);
        }

        for (const std::string& object : armorObjects)
        {
            std::string armorName = extractStringValue(object, "name", "Tenue simple");
            Armor fallback = armorFromName(armorName);

            Armor armor(
                armorName,
                extractStringValue(object, "description", fallback.getDescription()),
                extractIntValue(object, "value", fallback.getValue()),
                armorTypeFromSaveText(extractStringValue(object, "type", armorTypeToSaveText(fallback.getType()))),
                extractIntValue(object, "maxHpBonus", fallback.getMaxHpBonus()),
                extractIntValue(object, "damageReduction", fallback.getDamageReduction()),
                extractIntValue(object, "maxDurability", fallback.getMaxDurability())
            );

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
            std::string quality = extractStringValue(object, "quality", "normal");
            player.getInventory().addMaterial(MaterialCatalog::createById(id, quantity, quality));
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
        bossesKilled,
        pvpVictories,
        pvpDefeats
    );

    std::vector<std::string> loadedLethalEliminations;
    for (const std::string& object : lethalEliminationObjects)
    {
        std::string label = extractStringValue(object, "label", "");
        if (!label.empty())
        {
            loadedLethalEliminations.push_back(label);
        }
    }
    player.setLoadedPvpLethalEliminations(loadedLethalEliminations);

    std::vector<std::string> loadedRecentEquipmentUsage;
    for (const std::string& object : recentEquipmentUsageObjects)
    {
        std::string label = extractStringValue(object, "label", "");
        if (!label.empty())
        {
            loadedRecentEquipmentUsage.push_back(label);
        }
    }
    player.setLoadedRecentCombatEquipmentUsage(loadedRecentEquipmentUsage);

    std::vector<std::string> loadedPassiveSkills;
    for (const std::string& object : passiveSkillObjects)
    {
        std::string id = extractStringValue(object, "id", "");
        if (!id.empty())
        {
            loadedPassiveSkills.push_back(id);
        }
    }

    std::vector<std::string> loadedActiveSkills;
    for (const std::string& object : activeSkillObjects)
    {
        std::string id = extractStringValue(object, "id", "");
        if (!id.empty())
        {
            loadedActiveSkills.push_back(id);
        }
    }

    player.setLoadedSkillState(
        loadedPassiveSkills,
        loadedActiveSkills,
        extractIntValue(content, "daggerKillProgress", 0),
        extractIntValue(content, "bowKillProgress", 0),
        extractIntValue(content, "bareHandKillProgress", 0),
        extractIntValue(content, "staffKillProgress", 0)
    );

    std::vector<int> loadedUnlockedBossIds;
    for (const std::string& object : unlockedBossObjects)
    {
        int id = extractIntValue(object, "id", 0);
        if (id > 0)
        {
            loadedUnlockedBossIds.push_back(id);
        }
    }

    std::vector<int> loadedRecentBossIds;
    for (const std::string& object : recentBossObjects)
    {
        int id = extractIntValue(object, "id", 0);
        if (id > 0)
        {
            loadedRecentBossIds.push_back(id);
        }
    }

    player.setLoadedBossRegistry(loadedUnlockedBossIds, loadedRecentBossIds);

    Weapon loadedGrinkaWeapon = WeaponCatalog::createRustySword();
    bool hasLoadedGrinkaWeapon = false;
    if (!grinkaStolenWeaponObjects.empty())
    {
        const std::string& object = grinkaStolenWeaponObjects[0];
        std::string weaponName = extractStringValue(object, "name", "Épée rouillée");
        Weapon fallback = weaponFromName(weaponName);
        loadedGrinkaWeapon = Weapon(
            weaponName,
            extractStringValue(object, "description", fallback.getDescription()),
            extractIntValue(object, "value", fallback.getValue()),
            weaponTypeFromSaveText(extractStringValue(object, "type", weaponTypeToSaveText(fallback.getType()))),
            extractIntValue(object, "minDamageBonus", fallback.getMinDamageBonus()),
            extractIntValue(object, "maxDamageBonus", fallback.getMaxDamageBonus()),
            extractIntValue(object, "criticalBonus", fallback.getCriticalBonus()),
            extractIntValue(object, "maxDurability", fallback.getMaxDurability())
        );
        applySavedWeaponDurability(loadedGrinkaWeapon, extractIntValue(object, "durability", loadedGrinkaWeapon.getDurability()));
        hasLoadedGrinkaWeapon = true;
    }

    Armor loadedGrinkaArmor = ArmorCatalog::createSimpleOutfit();
    bool hasLoadedGrinkaArmor = false;
    if (!grinkaStolenArmorObjects.empty())
    {
        const std::string& object = grinkaStolenArmorObjects[0];
        std::string armorName = extractStringValue(object, "name", "Tenue simple");
        Armor fallback = armorFromName(armorName);
        loadedGrinkaArmor = Armor(
            armorName,
            extractStringValue(object, "description", fallback.getDescription()),
            extractIntValue(object, "value", fallback.getValue()),
            armorTypeFromSaveText(extractStringValue(object, "type", armorTypeToSaveText(fallback.getType()))),
            extractIntValue(object, "maxHpBonus", fallback.getMaxHpBonus()),
            extractIntValue(object, "damageReduction", fallback.getDamageReduction()),
            extractIntValue(object, "maxDurability", fallback.getMaxDurability())
        );
        applySavedArmorDurability(loadedGrinkaArmor, extractIntValue(object, "durability", loadedGrinkaArmor.getDurability()));
        hasLoadedGrinkaArmor = true;
    }

    player.setLoadedBossConsequences(
        extractBoolValue(content, "zelefCorrosionPresent", false),
        extractIntValue(content, "zelefMaxHpStolen", 0),
        extractBoolValue(content, "grinkaBossTheftPresent", false),
        extractIntValue(content, "grinkaStolenGold", 0),
        hasLoadedGrinkaWeapon,
        loadedGrinkaWeapon,
        hasLoadedGrinkaArmor,
        loadedGrinkaArmor
    );

    if (equippedWeaponIndex >= 0)
    {
        player.equipWeapon(equippedWeaponIndex);
    }

    if (equippedArmorIndex >= 0)
    {
        player.equipArmor(equippedArmorIndex);
    }

    player.setLoadedCombatStats(
        loadedMaxHp,
        loadedMinDamage,
        loadedMaxDamage,
        loadedCriticalDamage,
        hp
    );

    bool altered = extractBoolValue(content, "altered", false);
    bool godMode = extractBoolValue(content, "godMode", false);
    bool infiniteConsumables = extractBoolValue(content, "infiniteConsumables", false);
    bool indestructibleEquipment = extractBoolValue(content, "indestructibleEquipment", false);
    bool equipmentProtection = extractBoolValue(content, "equipmentProtection", false);
    bool storySkip = extractBoolValue(content, "storySkip", false);
    bool specialChallengeAccess = extractBoolValue(content, "specialChallengeAccess", false);

    player.setCheatState(
        altered,
        godMode,
        infiniteConsumables,
        indestructibleEquipment,
        equipmentProtection,
        storySkip,
        specialChallengeAccess,
        extractIntValue(content, "refundUsesRemaining", 3)
    );

    player.setCheatKnowledgeState(
        extractBoolValue(content, "godModeKnown", godMode),
        extractBoolValue(content, "infiniteConsumablesKnown", infiniteConsumables),
        extractBoolValue(content, "indestructibleEquipmentKnown", indestructibleEquipment),
        extractBoolValue(content, "equipmentProtectionKnown", equipmentProtection),
        extractBoolValue(content, "storySkipKnown", storySkip),
        extractBoolValue(content, "specialChallengeAccessKnown", specialChallengeAccess),
        extractBoolValue(content, "creatorMessageKnown", false)
    );

    player.setLethalCheatPenaltyState(
        extractIntValue(content, "lethalCheatAttemptCount", 0),
        extractIntValue(content, "worldGazeCombatsRemaining", 0),
        extractIntValue(content, "worldGazeMaxHpPenalty", 0)
    );

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
        player.setCheatState(false, false, false, false, false, false, false, player.getRefundUsesRemaining());
        player.setCheatKnowledgeState(false, false, false, false, false, false, false);
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

    player.getQuestLog().clear();

    for (const std::string& object : questObjects)
    {
        Quest quest;
        quest.id = extractStringValue(object, "id", "unknown_quest");
        quest.rank = extractStringValue(object, "rank", "F");
        quest.title = extractStringValue(object, "title", "Quête sauvegardée");
        quest.origin = extractStringValue(object, "origin", "Inconnue");
        quest.client = extractStringValue(object, "client", "Client inconnu");
        quest.location = extractStringValue(object, "location", "Lieu inconnu");
        quest.objective = extractStringValue(object, "objective", "Objectif sauvegardé.");
        quest.rewardExperience = extractIntValue(object, "rewardExperience", 0);
        quest.rewardGold = extractIntValue(object, "rewardGold", 0);
        quest.requiredMaterialId = extractStringValue(object, "requiredMaterialId", "");
        quest.requiredMaterialName = extractStringValue(object, "requiredMaterialName", "");
        quest.requiredMaterialQuantity = extractIntValue(object, "requiredMaterialQuantity", 0);
        quest.objectiveType = extractStringValue(object, "objectiveType", quest.requiredMaterialId.empty() ? "combat" : "livraison");
        quest.targetFamily = extractStringValue(object, "targetFamily", "Général");
        quest.progress = extractIntValue(object, "progress", 0);
        quest.target = extractIntValue(object, "target", 1);
        quest.guildQuest = extractBoolValue(object, "guildQuest", false);
        quest.accepted = extractBoolValue(object, "accepted", true);
        quest.completed = extractBoolValue(object, "completed", false);
        quest.turnedIn = extractBoolValue(object, "turnedIn", false);

        player.getQuestLog().getQuests().push_back(quest);
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

// EN: deletePlayableCharacter declares or implements a focused behavior used by this module.
// FR: deletePlayableCharacter déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: deleteAccountAndLinkedCharacters declares or implements a focused behavior used by this module.
// FR: deleteAccountAndLinkedCharacters déclare ou implémente un comportement précis utilisé par ce module.
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


// EN: exportAccountPackage declares or implements a focused behavior used by this module.
// FR: exportAccountPackage déclare ou implémente un comportement précis utilisé par ce module.
bool SaveManager::exportAccountPackage(const std::string& accountName, std::string& exportedPath)
{
    exportedPath.clear();

    if (!ensureSaveDirectories())
    {
        return false;
    }

    std::string safeAccount = buildSafeFileName(accountName);
    std::string accountPath = getAccountSavePath(accountName);

    if (!std::filesystem::exists(accountPath))
    {
        return false;
    }

    std::string packageRoot = SAVE_ROOT + "/exported_accounts/" + safeAccount + "_dinotofu_account";

    try
    {
        if (std::filesystem::exists(packageRoot))
        {
            std::filesystem::remove_all(packageRoot);
        }

        std::filesystem::create_directories(packageRoot + "/accounts");
        std::filesystem::create_directories(packageRoot + "/characters/playable");
        std::filesystem::create_directories(packageRoot + "/characters/dead");
        std::filesystem::create_directories(packageRoot + "/bestiary");
        std::filesystem::create_directories(packageRoot + "/materials");

        // Le compte local reste visible, mais ses personnages/donnees liees partent en voyage.
        std::filesystem::copy_file(
            accountPath,
            packageRoot + "/accounts/" + safeAccount + ".json",
            std::filesystem::copy_options::overwrite_existing
        );

        std::string accountPrefix = safeAccount + "__";

        std::vector<std::pair<std::string, std::string>> folders = {
            {SAVE_ROOT + "/characters/playable", packageRoot + "/characters/playable"},
            {SAVE_ROOT + "/characters/dead", packageRoot + "/characters/dead"},
            {SAVE_ROOT + "/bestiary", packageRoot + "/bestiary"},
            {SAVE_ROOT + "/materials", packageRoot + "/materials"}
        };

        for (const auto& folderPair : folders)
        {
            if (!std::filesystem::exists(folderPair.first))
            {
                continue;
            }

            for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(folderPair.first))
            {
                if (!entry.is_regular_file())
                {
                    continue;
                }

                std::string fileName = entry.path().filename().string();

                if (fileName.rfind(accountPrefix, 0) == 0 || fileName.rfind(safeAccount + ".", 0) == 0)
                {
                    std::string destination = folderPair.second + "/" + fileName;

                    if (std::filesystem::exists(destination))
                    {
                        std::filesystem::remove(destination);
                    }

                    std::filesystem::rename(entry.path(), destination);
                }
            }
        }

        std::ofstream manifest(packageRoot + "/LISEZ_MOI_IMPORT.txt");
        if (manifest.is_open())
        {
            manifest << "Export Dinotofu du compte : " << accountName << "\n";
            manifest << "Mode transfert : les personnages et donnees liees ont ete retires de cette installation.\n";
            manifest << "Le compte local reste present, mais il peut etre vide si tous ses personnages sont partis.\n";
            manifest << "Copie ce dossier sur cle USB puis importe-le depuis le menu des comptes.\n";
        }

        exportedPath = packageRoot;
        return true;
    }
    catch (...)
    {
        return false;
    }
}

// EN: exportCharacterPackage declares or implements a focused behavior used by this module.
// FR: exportCharacterPackage déclare ou implémente un comportement précis utilisé par ce module.
bool SaveManager::exportCharacterPackage(const CharacterSaveSummary& summary, std::string& exportedPath)
{
    exportedPath.clear();

    if (!ensureSaveDirectories())
    {
        return false;
    }

    if (summary.path.empty() || !std::filesystem::exists(summary.path))
    {
        return false;
    }

    std::string safeAccount = buildSafeFileName(summary.accountName);
    std::string safeCharacter = buildSafeFileName(summary.characterName);
    std::string accountPath = getAccountSavePath(summary.accountName);
    std::string packageRoot = SAVE_ROOT + "/exported_accounts/" + safeAccount + "__" + safeCharacter + "_dinotofu_character";

    try
    {
        if (std::filesystem::exists(packageRoot))
        {
            std::filesystem::remove_all(packageRoot);
        }

        std::filesystem::create_directories(packageRoot + "/accounts");
        std::filesystem::create_directories(packageRoot + "/characters/playable");
        std::filesystem::create_directories(packageRoot + "/characters/dead");
        std::filesystem::create_directories(packageRoot + "/bestiary");
        std::filesystem::create_directories(packageRoot + "/materials");

        if (std::filesystem::exists(accountPath))
        {
            std::filesystem::copy_file(
                accountPath,
                packageRoot + "/accounts/" + safeAccount + ".json",
                std::filesystem::copy_options::overwrite_existing
            );
        }

        std::string destination = packageRoot + "/characters/playable/" + std::filesystem::path(summary.path).filename().string();

        if (std::filesystem::exists(destination))
        {
            std::filesystem::remove(destination);
        }

        // Le personnage quitte vraiment cette installation : pas de duplication locale.
        std::filesystem::rename(summary.path, destination);

        std::ofstream manifest(packageRoot + "/LISEZ_MOI_IMPORT.txt");
        if (manifest.is_open())
        {
            manifest << "Export Dinotofu du personnage : " << summary.characterName << "\n";
            manifest << "Compte d'origine : " << summary.accountName << "\n";
            manifest << "Mode voyage : le personnage a ete retire du compte local.\n";
            manifest << "Importe ce dossier depuis le menu des comptes pour le recuperer.\n";
        }

        exportedPath = packageRoot;
        return true;
    }
    catch (...)
    {
        return false;
    }
}


// EN: exportCharacterClonePackage declares or implements a focused behavior used by this module.
// FR: exportCharacterClonePackage déclare ou implémente un comportement précis utilisé par ce module.
bool SaveManager::exportCharacterClonePackage(const CharacterSaveSummary& summary, std::string& exportedPath)
{
    exportedPath.clear();

    if (!ensureSaveDirectories())
    {
        return false;
    }

    if (summary.path.empty() || !std::filesystem::exists(summary.path))
    {
        return false;
    }

    std::string safeAccount = buildSafeFileName(summary.accountName);
    std::string safeCharacter = buildSafeFileName(summary.characterName);
    std::string accountPath = getAccountSavePath(summary.accountName);
    std::string packageRoot = SAVE_ROOT + "/exported_accounts/" + safeAccount + "__" + safeCharacter + "_dinotofu_clone";

    try
    {
        if (std::filesystem::exists(packageRoot))
        {
            std::filesystem::remove_all(packageRoot);
        }

        std::filesystem::create_directories(packageRoot + "/accounts");
        std::filesystem::create_directories(packageRoot + "/characters/playable");
        std::filesystem::create_directories(packageRoot + "/characters/dead");
        std::filesystem::create_directories(packageRoot + "/bestiary");
        std::filesystem::create_directories(packageRoot + "/materials");

        if (std::filesystem::exists(accountPath))
        {
            std::filesystem::copy_file(
                accountPath,
                packageRoot + "/accounts/" + safeAccount + ".json",
                std::filesystem::copy_options::overwrite_existing
            );
        }

        std::string cloneContent = withJsonBooleanField(readFileContent(summary.path), "clone", true);
        std::string destination = packageRoot + "/characters/playable/" + std::filesystem::path(summary.path).filename().string();

        if (!writeFileContent(destination, cloneContent))
        {
            return false;
        }

        std::ofstream manifest(packageRoot + "/LISEZ_MOI_IMPORT.txt");
        if (manifest.is_open())
        {
            manifest << "Clone Dinotofu du personnage : " << summary.characterName << "\n";
            manifest << "Compte d'origine : " << summary.accountName << "\n";
            manifest << "Mode clone : le personnage original reste sur ce compte.\n";
            manifest << "Un clone ne peut faire que des combats JcJ amicaux.\n";
            manifest << "Si le vrai personnage revient sur un compte qui possede ce clone, le clone est supprime.\n";
        }

        exportedPath = packageRoot;
        return true;
    }
    catch (...)
    {
        return false;
    }
}

// EN: importAccountPackage declares or implements a focused behavior used by this module.
// FR: importAccountPackage déclare ou implémente un comportement précis utilisé par ce module.
bool SaveManager::importAccountPackage(const std::string& packagePath, std::string& importedAccountName)
{
    importedAccountName.clear();

    if (!ensureSaveDirectories())
    {
        return false;
    }

    try
    {
        if (!std::filesystem::exists(packagePath) || !std::filesystem::is_directory(packagePath))
        {
            return false;
        }

        std::string accountFolder = packagePath + "/accounts";

        if (!std::filesystem::exists(accountFolder))
        {
            return false;
        }

        std::string detectedAccountPath;

        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(accountFolder))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".json")
            {
                detectedAccountPath = entry.path().string();
                break;
            }
        }

        if (detectedAccountPath.empty())
        {
            return false;
        }

        std::string accountContent = readFileContent(detectedAccountPath);
        importedAccountName = extractStringValue(accountContent, "accountName", std::filesystem::path(detectedAccountPath).stem().string());
        std::string safeAccount = buildSafeFileName(importedAccountName);

        std::filesystem::copy_file(
            detectedAccountPath,
            getAccountSavePath(importedAccountName),
            std::filesystem::copy_options::overwrite_existing
        );

        std::vector<std::pair<std::string, std::string>> folders = {
            {packagePath + "/characters/playable", SAVE_ROOT + "/characters/playable"},
            {packagePath + "/characters/dead", SAVE_ROOT + "/characters/dead"},
            {packagePath + "/bestiary", SAVE_ROOT + "/bestiary"},
            {packagePath + "/materials", SAVE_ROOT + "/materials"}
        };

        for (const auto& folderPair : folders)
        {
            if (!std::filesystem::exists(folderPair.first))
            {
                continue;
            }

            std::filesystem::create_directories(folderPair.second);

            for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(folderPair.first))
            {
                if (!entry.is_regular_file())
                {
                    continue;
                }

                std::string fileName = entry.path().filename().string();

                if (fileName.rfind(safeAccount + "__", 0) != 0 && fileName.rfind(safeAccount + ".", 0) != 0)
                {
                    continue;
                }

                std::string destination = folderPair.second + "/" + fileName;

                if (folderPair.second.find("/characters/playable") != std::string::npos)
                {
                    std::string incomingContent = readFileContent(entry.path().string());
                    bool incomingClone = extractBoolValue(incomingContent, "clone", false);
                    std::string deadMirror = SAVE_ROOT
                        + "/characters/dead/"
                        + std::filesystem::path(fileName).stem().string()
                        + "__dead.json";

                    if (std::filesystem::exists(deadMirror))
                    {
                        return false;
                    }

                    if (std::filesystem::exists(destination))
                    {
                        bool existingClone = extractBoolValue(readFileContent(destination), "clone", false);

                        if (incomingClone)
                        {
                            // Un clone ne peut pas revenir sur le compte si le vrai personnage existe deja.
                            return false;
                        }

                        if (!existingClone)
                        {
                            return false;
                        }

                        // Le vrai personnage revient : le clone local est supprime automatiquement.
                        std::filesystem::remove(destination);
                    }
                }

                if (folderPair.second.find("/characters/dead") != std::string::npos)
                {
                    std::string stem = std::filesystem::path(fileName).stem().string();
                    std::string playableStem = stem;
                    std::string deadSuffix = "__dead";

                    if (playableStem.size() > deadSuffix.size()
                        && playableStem.rfind(deadSuffix) == playableStem.size() - deadSuffix.size())
                    {
                        playableStem = playableStem.substr(0, playableStem.size() - deadSuffix.size());
                    }

                    std::string playableMirror = SAVE_ROOT + "/characters/playable/" + playableStem + ".json";

                    if (std::filesystem::exists(destination) || std::filesystem::exists(playableMirror))
                    {
                        return false;
                    }
                }

                std::filesystem::copy_file(
                    entry.path(),
                    destination,
                    std::filesystem::copy_options::overwrite_existing
                );
            }
        }

        return true;
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
