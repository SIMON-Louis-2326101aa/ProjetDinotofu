// EN: SaveManager.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SaveManager.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file prepares Dinotofu save helpers. Code identifiers stay in English; player-facing text can stay in French.
// Français : Ce fichier prépare les helpers de sauvegarde Dinotofu. Les identifiants restent en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Minimal JSON-like save manager for accounts, playable characters, future bestiary and materials.

#include "save/SaveManager.hpp"
#include "save/SaveSchemaVersion.hpp"
#include "core/VersionInfo.hpp"
#include "economy/Money.hpp"

#include "character/CharacterRace.hpp"
#include "class_system/ClassCatalog.hpp"
#include "item/armor/ArmorCatalog.hpp"
#include "item/consumable/ConsumableCatalog.hpp"
#include "item/consumable/Consumable.hpp"
#include "item/consumable/ConsumableType.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "item/weapon/WeaponCatalog.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"
#include "progression/material/MaterialKnowledgeProgress.hpp"
#include "progression/DeathRuleRules.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>

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

    std::string formatLastActivityText(const std::filesystem::file_time_type& fileTime)
    {
        try
        {
            const auto systemTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                fileTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
            );
            const std::time_t time = std::chrono::system_clock::to_time_t(systemTime);
            std::tm local{};
#if defined(_WIN32)
            localtime_s(&local, &time);
#else
            localtime_r(&time, &local);
#endif
            std::ostringstream out;
            out << std::put_time(&local, "%d/%m/%Y %H:%M");
            return out.str();
        }
        catch (...)
        {
            return "Inconnue";
        }
    }

    std::string formatLastActivityText(const std::string& path)
    {
        try
        {
            return formatLastActivityText(std::filesystem::last_write_time(path));
        }
        catch (...)
        {
            return "Inconnue";
        }
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
        file.flush();
        return file.good();
    }

    struct SaveFileRollbackState
    {
        std::string path;
        bool existed = false;
        std::string content;
    };

    SaveFileRollbackState captureSaveFileState(const std::string& path)
    {
        SaveFileRollbackState state;
        state.path = path;
        state.existed = std::filesystem::exists(path);
        if (state.existed)
        {
            state.content = readFileContent(path);
        }
        return state;
    }

    bool restoreSaveFileState(const SaveFileRollbackState& state)
    {
        try
        {
            if (state.existed)
            {
                return writeFileContent(state.path, state.content);
            }

            if (std::filesystem::exists(state.path))
            {
                std::filesystem::remove(state.path);
            }
            return true;
        }
        catch (...)
        {
            return false;
        }
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

    std::string withJsonStringField(
        const std::string& content,
        const std::string& key,
        const std::string& value,
        bool insideCharacter
    )
    {
        std::string pattern = "\"" + key + "\"";
        std::size_t keyPosition = content.find(pattern);
        std::string replacement = "\"" + key + "\": \"" + escapeJson(value) + "\"";

        if (keyPosition != std::string::npos)
        {
            std::size_t colonPosition = content.find(':', keyPosition);
            if (colonPosition == std::string::npos) return content;

            std::size_t valueStart = content.find_first_not_of(" \t\r\n", colonPosition + 1);
            if (valueStart == std::string::npos) return content;

            std::size_t valueEnd = valueStart;
            bool quoted = content[valueStart] == '"';
            if (quoted)
            {
                valueEnd = content.find('"', valueStart + 1);
                if (valueEnd == std::string::npos) return content;
                ++valueEnd;
            }
            else
            {
                while (valueEnd < content.size()
                       && content[valueEnd] != ','
                       && content[valueEnd] != '\n'
                       && content[valueEnd] != '}')
                {
                    ++valueEnd;
                }
            }

            std::string updated = content;
            updated.replace(keyPosition, valueEnd - keyPosition, replacement);
            return updated;
        }

        std::string updated = content;

        if (insideCharacter)
        {
            std::string characterPattern = "\"character\"";
            std::size_t characterPosition = updated.find(characterPattern);
            std::size_t openingBrace = updated.find('{', characterPosition);

            if (characterPosition == std::string::npos || openingBrace == std::string::npos)
            {
                return content;
            }

            updated.insert(openingBrace + 1, "\n    " + replacement + ",");
            return updated;
        }

        std::size_t openingBrace = updated.find('{');
        if (openingBrace == std::string::npos)
        {
            return content;
        }

        updated.insert(openingBrace + 1, "\n  " + replacement + ",");
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


    std::string recoveryMetadataJson(const Item& item)
    {
        return ", \"recoveryTraits\": \"" + escapeJson(item.getRecoveryTraitsSaveText())
            + "\", \"recoveryPriority\": " + std::to_string(item.getRecoveryPriority());
    }

    void loadRecoveryMetadata(Item& item, const std::string& object)
    {
        item.loadRecoveryMetadataFromSaveText(
            extractStringValue(object, "recoveryTraits", ""),
            extractIntValue(object, "recoveryPriority", 0));
    }


    long long extractLongLongValue(
        const std::string& content,
        const std::string& key,
        long long fallback
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
            return std::stoll(content.substr(start, end - start));
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

    std::vector<std::string> extractNamedStringObjectsFromArray(
        const std::string& content,
        const std::string& key
    )
    {
        std::vector<std::string> values;
        for (const std::string& object : extractObjectsFromArray(content, key))
        {
            const std::string value = extractStringValue(object, "name", "");
            if (!value.empty() && std::find(values.begin(), values.end(), value) == values.end())
            {
                values.push_back(value);
            }
        }
        return values;
    }

    std::vector<AccountUnlockedTitleInfo> extractAccountTitleObjects(const std::string& content)
    {
        std::vector<AccountUnlockedTitleInfo> values;
        for (const std::string& object : extractObjectsFromArray(content, "accountUnlockedTitles"))
        {
            AccountUnlockedTitleInfo info;
            info.name = extractStringValue(object, "name", "");
            info.unlockedAt = extractStringValue(object, "unlockedAt", "");
            info.characterName = extractStringValue(object, "characterName", "");
            if (info.name.empty()) continue;

            const auto existing = std::find_if(values.begin(), values.end(), [&](const AccountUnlockedTitleInfo& value) {
                return value.name == info.name;
            });
            if (existing == values.end()) values.push_back(info);
        }
        return values;
    }

    std::string withJsonAccountTitleArrayField(
        const std::string& content,
        const std::vector<AccountUnlockedTitleInfo>& values)
    {
        std::ostringstream replacement;
        replacement << "\"accountUnlockedTitles\": [";
        if (!values.empty()) replacement << "\n";
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            replacement << "    {\"name\": \"" << escapeJson(values[i].name) << "\"";
            if (!values[i].unlockedAt.empty())
            {
                replacement << ", \"unlockedAt\": \"" << escapeJson(values[i].unlockedAt) << "\"";
            }
            if (!values[i].characterName.empty())
            {
                replacement << ", \"characterName\": \"" << escapeJson(values[i].characterName) << "\"";
            }
            replacement << "}";
            if (i + 1 < values.size()) replacement << ",";
            replacement << "\n";
        }
        replacement << "  ]";

        const std::string key = "accountUnlockedTitles";
        const std::string pattern = "\"" + key + "\"";
        const std::size_t keyPosition = content.find(pattern);
        if (keyPosition != std::string::npos)
        {
            const std::size_t arrayStart = content.find('[', keyPosition);
            if (arrayStart == std::string::npos) return content;

            int depth = 0;
            std::size_t arrayEnd = std::string::npos;
            for (std::size_t i = arrayStart; i < content.size(); ++i)
            {
                if (content[i] == '[') ++depth;
                else if (content[i] == ']')
                {
                    --depth;
                    if (depth == 0)
                    {
                        arrayEnd = i;
                        break;
                    }
                }
            }
            if (arrayEnd == std::string::npos) return content;

            std::string updated = content;
            updated.replace(keyPosition, arrayEnd - keyPosition + 1, replacement.str());
            return updated;
        }

        const std::string insertionMarker = "  \"saveFolders\": {";
        const std::size_t insertionPosition = content.find(insertionMarker);
        if (insertionPosition == std::string::npos) return content;

        std::string updated = content;
        updated.insert(insertionPosition, "  " + replacement.str() + ",\n");
        return updated;
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
        if (normalized == normalizeText("Dague d'entraînement")) return WeaponCatalog::createTrainingDagger();
        if (normalized == normalizeText("Lance d'entraînement")) return WeaponCatalog::createTrainingSpear();
        if (normalized == normalizeText("Arc d'entraînement")) return WeaponCatalog::createTrainingBow();
        if (normalized == normalizeText("Arbalète d'entraînement")) return WeaponCatalog::createTrainingCrossbow();
        if (normalized == normalizeText("Bandoulière de lancer")) return WeaponCatalog::createTrainingThrowingBandolier();
        if (normalized == normalizeText("Couteau de bois d'urgence")) return WeaponCatalog::createEmergencyWoodKnife();
        if (normalized == normalizeText("Bâton d'apprenti")) return WeaponCatalog::createTrainingStaff();
        if (normalized == normalizeText("Hache lourde émoussée")) return WeaponCatalog::createHeavyTrainingAxe();
        if (normalized == normalizeText("Lame d'arène")) return WeaponCatalog::createArenaBlade();
        if (normalized == normalizeText("Fauchon de relais")) return WeaponCatalog::createRelayFalchion();
        if (normalized == normalizeText("Marteau de mine sifflante")) return WeaponCatalog::createWhistlingMineHammer();
        if (normalized == normalizeText("Bâton de résine chantante")) return WeaponCatalog::createSingingResinStaff();
        if (normalized == normalizeText("Arc des lanternes froides")) return WeaponCatalog::createColdLanternBow();
        if (normalized == normalizeText("Sabre d'argile rouge")) return WeaponCatalog::createRedClaySabre();
        if (normalized == normalizeText("Dague de carte brisée")) return WeaponCatalog::createBrokenMapDagger();
        if (normalized == normalizeText("Rapière des lucioles de fer")) return WeaponCatalog::createFireflyIronRapier();
        if (normalized == normalizeText("Masse du registre noyé")) return WeaponCatalog::createDrownedLedgerMace();
        if (normalized == normalizeText("Lance des falaises grises")) return WeaponCatalog::createGreyCliffSpear();
        if (normalized == normalizeText("Fouet de foire cassée")) return WeaponCatalog::createBrokenCarnivalWhip();
        if (normalized.find(normalizeText("Lame de récupération")) != std::string::npos)
        {
            bool special = normalized.find(normalizeText("particularité")) != std::string::npos;
            bool downgraded = normalized.find(normalizeText("haute qualité")) != std::string::npos
                || normalized.find(normalizeText("haute qualite")) != std::string::npos;
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

            if (downgraded)
            {
                return Weapon(
                    base.getName() + " - haute qualité abîmée",
                    base.getDescription() + " Qualité dégradée : l'arme était exceptionnelle, mais sa durabilité est tombée à 0 avant réparation. Elle reste haute qualité après réparation, sans récupérer son grade exceptionnel.",
                    std::max(1, (base.getValue() + 35) * 85 / 100),
                    base.getType(),
                    base.getMinDamageBonus() + 1,
                    base.getMaxDamageBonus() + 2,
                    base.getCriticalBonus() + 1,
                    base.getMaxDurability() + 8
                );
            }

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
        if (normalized == normalizeText("Robe d'apprenti renforcée")) return ArmorCatalog::createApprenticeRobe();
        if (normalized == normalizeText("Veste matelassée de départ")) return ArmorCatalog::createPaddedVest();
        if (normalized == normalizeText("Armure lourde rafistolée")) return ArmorCatalog::createHeavyPaddedArmor();
        if (normalized == normalizeText("Cotte de maille d'arène")) return ArmorCatalog::createArenaChainmail();
        if (normalized == normalizeText("Manteau de cartographe abîmé")) return ArmorCatalog::createDamagedCartographerCoat();
        if (normalized == normalizeText("Plastron d'argile séchée")) return ArmorCatalog::createSunDriedClayBreastplate();
        if (normalized == normalizeText("Robe aux fibres vivantes")) return ArmorCatalog::createLivingFiberRobe();
        if (normalized == normalizeText("Harnais de mineur sifflant")) return ArmorCatalog::createWhistlingMinerHarness();
        if (normalized == normalizeText("Gilet d'archiviste noyé")) return ArmorCatalog::createDrownedArchivistVest();
        if (normalized == normalizeText("Harnais de drake gris")) return ArmorCatalog::createGreyDrakeHarness();
        if (normalized == normalizeText("Cape de foire décousue")) return ArmorCatalog::createPatchworkCarnivalCape();
        if (normalized == normalizeText("Parka de survie glaciale")) return ArmorCatalog::createColdSurvivalParka();
        if (normalized == normalizeText("Tenue ignifugée de terrain")) return ArmorCatalog::createHeatSurvivalSuit();
        if (normalized == normalizeText("Manteau isolant d'explorateur")) return ArmorCatalog::createInsulatedExplorerCoat();
        if (normalized.find(normalizeText("Armure de chasseur rafistolée")) != std::string::npos)
        {
            bool special = normalized.find(normalizeText("particularité")) != std::string::npos;
            bool downgraded = normalized.find(normalizeText("haute qualité")) != std::string::npos
                || normalized.find(normalizeText("haute qualite")) != std::string::npos;
            Armor base(
                "Armure de chasseur rafistolée",
                "Une armure souple faite pour survivre aux mauvaises rencontres plutôt que briller dans une forge royale.",
                90,
                ArmorType::Leather,
                38,
                4,
                125
            );

            if (downgraded)
            {
                return Armor(
                    base.getName() + " - haute qualité abîmée",
                    base.getDescription() + " Qualité dégradée : l'armure était exceptionnelle, mais sa durabilité est tombée à 0 avant réparation. Elle reste haute qualité après réparation, sans récupérer son grade exceptionnel.",
                    std::max(1, (base.getValue() + 40) * 85 / 100),
                    base.getType(),
                    base.getMaxHpBonus() + 6,
                    base.getDamageReduction() + 1,
                    base.getMaxDurability() + 10
                );
            }

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

        if (normalized == normalizeText("Petite potion de soin")) return ConsumableCatalog::createMinorHealingPotion();
        if (normalized == normalizeText("Potion de soin")) return ConsumableCatalog::createBasicHealingPotion();
        if (normalized == normalizeText("Potion de mana")) return ConsumableCatalog::createManaPotion();
        if (normalized == normalizeText("Potion de soin renforcée")) return ConsumableCatalog::createReinforcedHealingPotion();
        if (normalized == normalizeText("Potion de soin supérieure")) return ConsumableCatalog::createGreaterHealingPotion();
        if (normalized == normalizeText("Potion de soin majeure")) return ConsumableCatalog::createMajorHealingPotion();
        if (normalized == normalizeText("Potion de vitalité proportionnelle")) return ConsumableCatalog::createVitalityHealingPotion();
        if (normalized == normalizeText("Potion de vitalité royale")) return ConsumableCatalog::createRoyalVitalityHealingPotion();
        if (normalized == normalizeText("Potion d'élan proportionnel")) return ConsumableCatalog::createMomentumDamagePotion();
        if (normalized == normalizeText("Potion de garde proportionnelle")) return ConsumableCatalog::createGuardianPercentPotion();
        if (normalized == normalizeText("Fiole de rupture proportionnelle")) return ConsumableCatalog::createProportionalFragilityDebuffPotion();
        if (normalized == normalizeText("Petite potion de rage")) return ConsumableCatalog::createMinorDamagePotion();
        if (normalized == normalizeText("Potion de rage")) return ConsumableCatalog::createBasicDamagePotion();
        if (normalized == normalizeText("Potion de rage supérieure")) return ConsumableCatalog::createReinforcedDamagePotion();
        if (normalized == normalizeText("Potion de rage majeure")) return ConsumableCatalog::createGreaterDamagePotion();
        if (normalized == normalizeText("Potion de rage expérimentale")) return ConsumableCatalog::createExperimentalDamagePotion();
        if (normalized == normalizeText("Potion défensive")) return ConsumableCatalog::createDefensivePotion();
        if (normalized == normalizeText("Potion défensive supérieure")) return ConsumableCatalog::createGreaterDefensivePotion();
        if (normalized == normalizeText("Potion de précision")) return ConsumableCatalog::createPrecisionPotion();
        if (normalized == normalizeText("Fiole d'affaiblissement")) return ConsumableCatalog::createWeakeningDebuffPotion();
        if (normalized == normalizeText("Fiole de fragilisation")) return ConsumableCatalog::createFragilityDebuffPotion();
        if (normalized == normalizeText("Antidote simple")) return ConsumableCatalog::createAntidotePotion();
        if (normalized == normalizeText("Baume anti-brûlure")) return ConsumableCatalog::createBurnSalvePotion();
        if (normalized == normalizeText("Potion tiède anti-givre")) return ConsumableCatalog::createFrostResistancePotion();
        if (normalized == normalizeText("Potion isolante")) return ConsumableCatalog::createShockResistancePotion();
        if (normalized == normalizeText("Potion de voile élémentaire")) return ConsumableCatalog::createElementalWardPotion();
        if (normalized == normalizeText("Thé stabilisant")) return ConsumableCatalog::createStabilizingTea();
        if (normalized == normalizeText("Tonique de mineur")) return ConsumableCatalog::createMinerBracingTonic();
        if (normalized == normalizeText("Encre de concentration")) return ConsumableCatalog::createCartographerFocusInk();
        if (normalized == normalizeText("Purificateur au sel lunaire")) return ConsumableCatalog::createMoonSaltPurifier();
        if (normalized == normalizeText("Parchemin de pas de verre")) return ConsumableCatalog::createGlassStepScroll();
        if (normalized == normalizeText("Parchemin de ronce-lien")) return ConsumableCatalog::createVineSnareScroll();
        if (normalized == normalizeText("Encre de concentration d'archiviste")) return ConsumableCatalog::createArchivistFocusInk();
        if (normalized == normalizeText("Tisane de basilic des falaises")) return ConsumableCatalog::createCliffBasilTea();
        if (normalized == normalizeText("Ticket de diversion de foire")) return ConsumableCatalog::createCarnivalDiversionTicket();
        if (normalized == normalizeText("Fiole de garde-lucioles")) return ConsumableCatalog::createFireflyGuardVial();
        if (normalized == normalizeText("Lucky Potion")) return ConsumableCatalog::createLuckyPotion();
        if (normalized == normalizeText("Unlucky Potion")) return ConsumableCatalog::createUnluckyPotion();
        if (normalized == normalizeText("Fiole de fumée de secours")) return ConsumableCatalog::createSmokeEscapeVial();
        if (normalized == normalizeText("Parchemin d'étincelle arcanique")) return ConsumableCatalog::createArcaneSparkScroll();
        if (normalized == normalizeText("Parchemin de voile élémentaire")) return ConsumableCatalog::createElementalWardScroll();
        if (normalized == normalizeText("Parchemin de faille fragile")) return ConsumableCatalog::createResistanceRiftScroll();
        if (normalized == normalizeText("Parchemin de braise errante")) return ConsumableCatalog::createWanderingEmberScroll();
        if (normalized == normalizeText("Parchemin de purification mineure")) return ConsumableCatalog::createMinorPurificationScroll();
        if (normalized == normalizeText("Parchemin de purification volé")) return ConsumableCatalog::createMinorPurificationScroll();
        if (normalized == normalizeText("Parchemin de venin rampant")) return ConsumableCatalog::createCrawlingVenomScroll();
        if (normalized == normalizeText("Parchemin de venin rampant interdit")) return ConsumableCatalog::createCrawlingVenomScroll();

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
            CharacterRace::SemiWolf,
            CharacterRace::SemiFox,
            CharacterRace::SemiDog,
            CharacterRace::SemiCat,
            CharacterRace::SemiLizard,
            CharacterRace::SemiBird,
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
        summary.creatorAccountName = extractStringValue(content, "creatorAccount", summary.accountName);
        summary.currentOwnerAccountName = extractStringValue(content, "currentOwnerAccount", summary.accountName);
        summary.characterName = extractStringValue(content, "name", "Inconnu");
        summary.raceName = extractStringValue(content, "race", "Humain");
        summary.characterAge = extractIntValue(content, "characterAge", 18);
        summary.visualPresentation = extractStringValue(content, "visualPresentation", "Non précisé");
        summary.visualVariant = extractStringValue(content, "visualVariant", "Variante A");
        summary.className = extractStringValue(content, "class", "Chevalier");
        summary.difficulty = difficultyFromText(extractStringValue(content, "difficulty", "Normal"));
        summary.deathRule = DeathRuleRules::fromSaveText(
            extractStringValue(content, "deathRule", DeathRuleRules::toSaveText(DeathRuleRules::defaultForDifficulty(summary.difficulty))),
            summary.difficulty
        );
        summary.level = extractIntValue(content, "level", 1);
        summary.clone = extractBoolValue(content, "clone", false);
        summary.gameVersion = extractStringValue(content, "gameVersion", "inconnue");
        summary.createdAt = extractStringValue(content, "createdAt", "Inconnue");
        summary.realPlayTimeSeconds = extractLongLongValue(content, "realPlayTimeSeconds", 0);
        summary.totalCopperCurrency = extractLongLongValue(content, "totalCopperCurrency", 0);
        summary.worldDaysElapsed = std::max(0, extractIntValue(content, "worldDaysElapsed", 0));
        summary.worldDayProgressUnits = std::clamp(extractIntValue(content, "worldDayProgressUnits", 0), 0, 4);
        summary.lastSavedAt = extractStringValue(content, "lastSavedAt", "");
        summary.createdForVersion = extractStringValue(content, "createdForVersion", summary.gameVersion);
        summary.lastAdaptedVersion = extractStringValue(content, "lastAdaptedVersion", summary.createdForVersion);
        summary.storyModeStarted = extractBoolValue(content, "storyModeStarted", false);
        summary.storyChapter = extractIntValue(content, "storyChapter", 0);
        summary.storyStep = extractIntValue(content, "storyStep", 0);
        summary.lastActivityText = summary.lastSavedAt.empty() ? formatLastActivityText(path) : summary.lastSavedAt;

        return summary;
    }
}

// EN: ensureSaveDirectories declares or implements a focused behavior used by this module.
// FR: ensureSaveDirectories déclare ou implémente un comportement précis utilisé par ce module.

int SaveManager::deleteEphemeralStoryCloneSaves()
{
    if (!ensureSaveDirectories())
    {
        return 0;
    }

    int deleted = 0;
    const std::string characterFolder = SAVE_ROOT + "/characters/playable";

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(characterFolder))
        {
            if (!entry.is_regular_file() || entry.path().extension() != ".json")
            {
                continue;
            }

            const std::string content = readFileContent(entry.path().string());
            const bool explicitEphemeral = content.find("\"storyEphemeralSandboxClone\": true") != std::string::npos;
            const bool legacyEphemeralName = content.find("__story_ephemeral_clone") != std::string::npos
                || content.find("[clone éphémère histoire]") != std::string::npos;

            if (explicitEphemeral || legacyEphemeralName)
            {
                std::filesystem::remove(entry.path());
                ++deleted;
            }
        }
    }
    catch (...)
    {
        return deleted;
    }

    return deleted;
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
    const std::string previousAccountContent = readFileContent(savePath);
    const long long preservedAccountPlaySeconds = extractLongLongValue(
        previousAccountContent, "totalRealPlayTimeSeconds", 0
    );
    const std::vector<AccountUnlockedTitleInfo> preservedUnlockedTitles = extractAccountTitleObjects(previousAccountContent);
    createBackupIfSaveExists(savePath);

    std::ofstream file(savePath);

    if (!file.is_open())
    {
        return false;
    }

    file << "{\n";
    file << "  \"saveVersion\": " << SaveSchemaVersion::Current << ",\n";
    file << "  \"gameVersion\": \"" << escapeJson(VersionInfo::currentVersion()) << "\",\n";
    file << "  \"versionPolicy\": \"X=phase majeure, Y=ajout/changement important, Z=correctif mineur\",\n";
    file << "  \"backupPolicy\": \"previous_save_written_to_bak_when_possible\",\n";
    file << "  \"accountName\": \"" << escapeJson(accountName) << "\",\n";
    file << "  \"status\": \"active\",\n";
    file << "  \"totalRealPlayTimeSeconds\": " << std::max(0LL, preservedAccountPlaySeconds) << ",\n";
    file << "  \"accountUnlockedTitles\": [\n";
    for (std::size_t i = 0; i < preservedUnlockedTitles.size(); ++i)
    {
        file << "    {\"name\": \"" << escapeJson(preservedUnlockedTitles[i].name) << "\"";
        if (!preservedUnlockedTitles[i].unlockedAt.empty())
        {
            file << ", \"unlockedAt\": \"" << escapeJson(preservedUnlockedTitles[i].unlockedAt) << "\"";
        }
        if (!preservedUnlockedTitles[i].characterName.empty())
        {
            file << ", \"characterName\": \"" << escapeJson(preservedUnlockedTitles[i].characterName) << "\"";
        }
        file << "}";
        if (i + 1 < preservedUnlockedTitles.size()) file << ",";
        file << "\n";
    }
    file << "  ],\n";
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

std::vector<std::string> SaveManager::getAccountUnlockedTitles(const std::string& accountName)
{
    std::vector<std::string> names;
    for (const AccountUnlockedTitleInfo& info : getAccountUnlockedTitleDetails(accountName))
    {
        if (!info.name.empty()) names.push_back(info.name);
    }
    return names;
}

std::vector<AccountUnlockedTitleInfo> SaveManager::getAccountUnlockedTitleDetails(const std::string& accountName)
{
    return extractAccountTitleObjects(readFileContent(getAccountSavePath(accountName)));
}

bool SaveManager::mergeAccountUnlockedTitles(
    const std::string& accountName,
    const std::vector<std::string>& titles,
    const std::string& characterName)
{
    if (accountName.empty() || !ensureSaveDirectories())
    {
        return false;
    }

    const std::string accountPath = getAccountSavePath(accountName);
    if (!std::filesystem::exists(accountPath) && !saveAccountSnapshot(accountName))
    {
        return false;
    }

    std::string content = readFileContent(accountPath);
    std::vector<AccountUnlockedTitleInfo> merged = extractAccountTitleObjects(content);
    bool changed = false;

    for (const std::string& title : titles)
    {
        if (title.empty()) continue;
        const auto found = std::find_if(merged.begin(), merged.end(), [&](const AccountUnlockedTitleInfo& info) {
            return info.name == title;
        });
        if (found != merged.end()) continue;

        AccountUnlockedTitleInfo info;
        info.name = title;
        info.unlockedAt = VersionInfo::currentDateTimeText();
        info.characterName = characterName;
        merged.push_back(info);
        changed = true;
    }

    if (!changed)
    {
        return true;
    }

    std::sort(merged.begin(), merged.end(), [](const AccountUnlockedTitleInfo& a, const AccountUnlockedTitleInfo& b) {
        return a.name < b.name;
    });
    content = withJsonAccountTitleArrayField(content, merged);
    return writeFileContent(accountPath, content);
}

bool SaveManager::savePlayerSnapshot(
    const Player& player,
    const std::string& accountName,
    DifficultyMode difficulty
)
{
    return savePlayerSnapshot(player, accountName, difficulty, DeathRuleRules::defaultForDifficulty(difficulty));
}

bool SaveManager::savePlayerSnapshot(
    const Player& player,
    const std::string& accountName,
    DifficultyMode difficulty,
    DeathRuleMode deathRule
)
{
    deathRule = DeathRuleRules::normalizeForDifficulty(difficulty, deathRule);
    if (!ensureSaveDirectories())
    {
        return false;
    }

    std::string savePath = getCharacterSavePath(accountName, player.getName());
    const long long previousCharacterPlaySeconds = extractLongLongValue(
        readFileContent(savePath), "realPlayTimeSeconds", 0
    );
    createBackupIfSaveExists(savePath);

    std::ofstream file(savePath);

    if (!file.is_open())
    {
        return false;
    }

    Weapon weapon = player.getEquippedWeapon();
    Armor armor = player.getEquippedArmor();
    std::string creatorAccount = player.getCreatorAccountName().empty()
        ? accountName
        : player.getCreatorAccountName();

    file << "{\n";
    file << "  \"saveVersion\": " << SaveSchemaVersion::Current << ",\n";
    file << "  \"gameVersion\": \"" << escapeJson(VersionInfo::currentVersion()) << "\",\n";
    file << "  \"versionPolicy\": \"X=phase majeure, Y=ajout/changement important, Z=correctif mineur\",\n";
    file << "  \"backupPolicy\": \"previous_save_written_to_bak_when_possible\",\n";
    file << "  \"account\": \"" << escapeJson(accountName) << "\",\n";
    file << "  \"creatorAccount\": \"" << escapeJson(creatorAccount) << "\",\n";
    file << "  \"currentOwnerAccount\": \"" << escapeJson(accountName) << "\",\n";
    file << "  \"difficulty\": \"" << escapeJson(difficultyToText(difficulty)) << "\",\n";
    file << "  \"deathRule\": \"" << escapeJson(DeathRuleRules::toSaveText(deathRule)) << "\",\n";
    file << "  \"character\": {\n";
    file << "    \"createdAt\": \"" << escapeJson(player.getCreatedAtText()) << "\",\n";
    file << "    \"lastSavedAt\": \"" << escapeJson(VersionInfo::currentDateTimeText()) << "\",\n";
    file << "    \"realPlayTimeSeconds\": " << player.getRealPlayTimeSeconds() << ",\n";
    file << "    \"creatorAccount\": \"" << escapeJson(creatorAccount) << "\",\n";
    file << "    \"currentOwnerAccount\": \"" << escapeJson(accountName) << "\",\n";
    file << "    \"createdForVersion\": \"" << escapeJson(player.getCreatedForVersion()) << "\",\n";
    file << "    \"lastAdaptedVersion\": \"" << escapeJson(player.getLastAdaptedVersion()) << "\",\n";
    file << "    \"name\": \"" << escapeJson(player.getName()) << "\",\n";
    file << "    \"race\": \"" << escapeJson(player.getRaceText()) << "\",\n";
    file << "    \"characterAge\": " << player.getCharacterAge() << ",\n";
    file << "    \"visualPresentation\": \"" << escapeJson(player.getVisualPresentation()) << "\",\n";
    file << "    \"visualVariant\": \"" << escapeJson(player.getVisualVariant()) << "\",\n";
    file << "    \"bodyMorphology\": \"" << escapeJson(player.getBodyMorphology()) << "\",\n";
    file << "    \"morphologyProgramTarget\": \"" << escapeJson(player.getMorphologyProgramTarget()) << "\",\n";
    file << "    \"morphologyProgramSessions\": " << player.getMorphologyProgramSessions() << ",\n";
    file << "    \"morphologyProgramStartedDay\": " << player.getMorphologyProgramStartedDay() << ",\n";
    file << "    \"morphologyProgramLastSessionDay\": " << player.getMorphologyProgramLastSessionDay() << ",\n";
    file << "    \"class\": \"" << escapeJson(player.getType()) << "\",\n";
    file << "    \"clone\": " << (player.isClone() ? "true" : "false") << ",\n";
    file << "    \"level\": " << player.getLevel() << ",\n";
    file << "    \"experience\": " << player.getExperience() << ",\n";
    file << "    \"hp\": " << player.getHp() << ",\n";
    file << "    \"maxHp\": " << player.getMaxHp() << ",\n";
    file << "    \"mana\": " << player.getMana() << ",\n";
    file << "    \"manaExhaustionTurns\": " << player.getManaExhaustionTurns() << ",\n";
    file << "    \"minDamage\": " << player.getMinDamage() << ",\n";
    file << "    \"maxDamage\": " << player.getMaxDamage() << ",\n";
    file << "    \"criticalDamage\": " << player.getCriticalDamage() << ",\n";
    file << "    \"gold\": " << player.getInventory().getGold() << ",\n";
    file << "    \"totalCopperCurrency\": " << player.getInventory().getTotalCopper() << ",\n";
    const CoinBreakdown currencyStacks = player.getInventory().getCoinStacks();
    file << "    \"currencyCopperCoins\": " << currencyStacks.copper << ",\n";
    file << "    \"currencyIronCoins\": " << currencyStacks.iron << ",\n";
    file << "    \"currencyElectrumCoins\": " << currencyStacks.electrum << ",\n";
    file << "    \"currencyGoldCoins\": " << currencyStacks.gold << ",\n";
    file << "    \"currencyPlatinumCoins\": " << currencyStacks.platinum << ",\n";
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
    file << "    \"equippedArmorName\": \"" << escapeJson(armor.getName()) << "\",\n";
    file << "    \"interfaceHintFrequency\": \"" << escapeJson(player.getInterfaceHintFrequency()) << "\",\n";
    file << "    \"revealHiddenTitles\": " << (player.areHiddenTitlesRevealed() ? "true" : "false") << ",\n";
    file << "    \"graphicalImagesEnabled\": " << (player.areGraphicalImagesEnabled() ? "true" : "false") << ",\n";
    file << "    \"imagePolicy\": \"images_supplementary_no_hidden_information_terminal_disabled\",\n";
    file << "    \"storyModeStarted\": " << (player.hasStoryModeStarted() ? "true" : "false") << ",\n";
    file << "    \"storyChapter\": " << player.getStoryChapter() << ",\n";
    file << "    \"storyStep\": " << player.getStoryStep() << ",\n";
    file << "    \"storyCityDevelopmentLevel\": " << player.getStoryCityDevelopmentLevel() << ",\n";
    const PlayerHomeState& homeState = player.getHomeState();
    file << "    \"homeOwned\": " << (homeState.owned ? "true" : "false") << ",\n";
    file << "    \"homeCityId\": \"" << escapeJson(homeState.cityId) << "\",\n";
    file << "    \"homeFullyPurchased\": " << (homeState.fullyPurchased ? "true" : "false") << ",\n";
    file << "    \"homeRentAmount\": " << homeState.rentAmount << ",\n";
    file << "    \"homeNextRentDueDay\": " << homeState.nextRentDueDay << ",\n";
    file << "    \"homeNpcSpouseName\": \"" << escapeJson(homeState.npcSpouseName) << "\",\n";
    file << "    \"homePlayerSpouseId\": \"" << escapeJson(homeState.playerSpouseId) << "\",\n";
    file << "    \"homePlayerMarriageSlotUsed\": " << (homeState.playerMarriageSlotUsed ? "true" : "false") << ",\n";
    const PlayerHestiaStoryState& hestiaState = player.getHestiaStoryState();
    file << "    \"hestiaRescued\": " << (hestiaState.rescued ? "true" : "false") << ",\n";
    file << "    \"hestiaMaterialized\": " << (hestiaState.materialized ? "true" : "false") << ",\n";
    file << "    \"hestiaResidence\": \"" << escapeJson(hestiaState.residence) << "\",\n";
    file << "    \"hestiaTrust\": " << hestiaState.trust << ",\n";
    file << "    \"hestiaAwakeningStage\": " << hestiaState.awakeningStage << ",\n";
    file << "    \"hestiaLastVisitDay\": " << hestiaState.lastVisitDay << ",\n";
    file << "    \"hestiaNextSmallFindDay\": " << hestiaState.nextSmallFindDay << ",\n";
    file << "    \"hestiaSacrificeConsumed\": " << (hestiaState.sacrificeConsumed ? "true" : "false") << ",\n";
    const PlayerHestiaArtifactState& hestiaArtifact = player.getHestiaArtifactState();
    file << "    \"hestiaArtifactGranted\": " << (hestiaArtifact.granted ? "true" : "false") << ",\n";
    file << "    \"hestiaArtifactLevel\": " << hestiaArtifact.level << ",\n";
    file << "    \"hestiaArtifactKillsTowardNextLevel\": " << hestiaArtifact.killsTowardNextLevel << ",\n";
    file << "    \"hestiaArtifactPrimaryForm\": \"" << escapeJson(hestiaArtifact.primaryForm) << "\",\n";
    file << "    \"hestiaArtifactSecondaryForm\": \"" << escapeJson(hestiaArtifact.secondaryForm) << "\",\n";
    file << "    \"hestiaArtifactActiveForm\": \"" << escapeJson(hestiaArtifact.activeForm) << "\",\n";
    file << "    \"hestiaArtifactSecondFormUnlocked\": " << (hestiaArtifact.secondFormUnlocked ? "true" : "false") << ",\n";
    file << "    \"hestiaArtifactEssences\": [";
    for (std::size_t i = 0; i < hestiaArtifact.absorbedEssences.size(); ++i)
    {
        const HestiaArtifactEssenceRecord& essence = hestiaArtifact.absorbedEssences[i];
        file << "{\"familyId\":\"" << escapeJson(essence.familyId)
             << "\",\"acceptedKills\":" << essence.acceptedKills << "}";
        if (i + 1 < hestiaArtifact.absorbedEssences.size()) file << ", ";
    }
    file << "]\n";
    file << "  },\n";

    file << "  \"selectionPreferences\": {\n";
    file << "    \"ammunitionSelectionPolicy\": \"" << escapeJson(player.getAmmunitionSelectionPolicy()) << "\",\n";
    file << "    \"magicMaterialSelectionPolicy\": \"" << escapeJson(player.getMagicMaterialSelectionPolicy()) << "\",\n";
    file << "    \"lootPickupPolicy\": \"" << escapeJson(player.getLootPickupPolicy()) << "\",\n";
    file << "    \"ignoredLootCategoryIds\": [";
    const std::vector<std::string>& ignoredLootCategoryIds = player.getIgnoredLootCategoryIds();
    for (std::size_t i = 0; i < ignoredLootCategoryIds.size(); ++i)
    {
        file << "{\"id\": \"" << escapeJson(ignoredLootCategoryIds[i]) << "\"}";
        if (i + 1 < ignoredLootCategoryIds.size()) file << ", ";
    }
    file << "],\n";
    file << "    \"ignoredLootMaterialIds\": [";
    const std::vector<std::string>& ignoredLootMaterialIds = player.getIgnoredLootMaterialIds();
    for (std::size_t i = 0; i < ignoredLootMaterialIds.size(); ++i)
    {
        file << "{\"id\": \"" << escapeJson(ignoredLootMaterialIds[i]) << "\"}";
        if (i + 1 < ignoredLootMaterialIds.size()) file << ", ";
    }
    file << "]\n";
    file << "  },\n";

    file << "  \"inventoryPreferences\": {\n";
    file << "    \"pinnedItemKeys\": [";
    const std::vector<std::string>& pinnedItemKeys = player.getInventory().getPinnedItemKeys();
    for (std::size_t i = 0; i < pinnedItemKeys.size(); ++i)
    {
        file << "{\"key\": \"" << escapeJson(pinnedItemKeys[i]) << "\"}";
        if (i + 1 < pinnedItemKeys.size()) file << ", ";
    }
    file << "],\n";
    file << "    \"favoriteAmmunitionIds\": [";
    const std::vector<std::string>& favoriteAmmunitionIds = player.getInventory().getFavoriteAmmunitionIds();
    for (std::size_t i = 0; i < favoriteAmmunitionIds.size(); ++i)
    {
        file << "{\"id\": \"" << escapeJson(favoriteAmmunitionIds[i]) << "\"}";
        if (i + 1 < favoriteAmmunitionIds.size()) file << ", ";
    }
    file << "],\n";
    file << "    \"lockedItemKeys\": [";
    const std::vector<std::string>& lockedItemKeys = player.getInventory().getLockedItemKeys();
    for (std::size_t i = 0; i < lockedItemKeys.size(); ++i)
    {
        file << "{\"key\": \"" << escapeJson(lockedItemKeys[i]) << "\"}";
        if (i + 1 < lockedItemKeys.size()) file << ", ";
    }
    file << "],\n";
    file << "    \"trackedMaterialIds\": [";
    const std::vector<std::string>& trackedMaterialIds = player.getInventory().getTrackedMaterialIds();
    for (std::size_t i = 0; i < trackedMaterialIds.size(); ++i)
    {
        file << "{\"id\": \"" << escapeJson(trackedMaterialIds[i]) << "\"}";
        if (i + 1 < trackedMaterialIds.size()) file << ", ";
    }
    file << "]\n";
    file << "  },\n";

    file << "  \"titleSnapshot\": {\n";
    file << "    \"activeTitle\": \"" << escapeJson(player.getActiveTitle()) << "\",\n";
    file << "    \"activeTitles\": [\n";
    const std::vector<std::string>& playerActiveTitles = player.getActiveTitles();
    for (std::size_t i = 0; i < playerActiveTitles.size(); ++i)
    {
        file << "      {\"name\": \"" << escapeJson(playerActiveTitles[i]) << "\"}";
        if (i + 1 < playerActiveTitles.size())
        {
            file << ",";
        }
        file << "\n";
    }
    file << "    ],\n";
    file << "    \"titles\": [\n";
    const std::vector<std::string>& playerTitles = player.getTitles();
    for (std::size_t i = 0; i < playerTitles.size(); ++i)
    {
        file << "      {\"name\": \"" << escapeJson(playerTitles[i]) << "\"}";
        if (i + 1 < playerTitles.size())
        {
            file << ",";
        }
        file << "\n";
    }
    file << "    ]\n";
    file << "  },\n";

    file << "  \"characterStatistics\": {\n";
    file << "    \"combatsStarted\": " << player.getCombatsStarted() << ",\n";
    file << "    \"worldDaysElapsed\": " << player.getWorldDaysElapsed() << ",\n";
    file << "    \"worldDayProgressUnits\": " << player.getWorldDayProgressUnits() << ",\n";
    file << "    \"worldDayUnitsPerDay\": " << player.getWorldDayUnitsPerDay() << ",\n";
    file << "    \"localSubscriptionRenewalPaidThisWeek\": " << player.getLocalSubscriptionRenewalPaidThisWeek() << ",\n";
    file << "    \"victories\": " << player.getVictories() << ",\n";
    file << "    \"defeats\": " << player.getDefeats() << ",\n";
    file << "    \"escapes\": " << player.getEscapes() << ",\n";
    file << "    \"deaths\": " << player.getDeaths() << ",\n";
    file << "    \"enemiesKilled\": " << player.getEnemiesKilled() << ",\n";
    file << "    \"bossesKilled\": " << player.getBossesKilled() << ",\n";
    file << "    \"pvpVictories\": " << player.getPvpVictories() << ",\n";
    file << "    \"pvpDefeats\": " << player.getPvpDefeats() << "\n";
    file << "  },\n";

    file << "  \"localSubscriptions\": [\n";
    const std::vector<PlayerLocalSubscription>& localSubscriptions = player.getLocalSubscriptions();
    for (std::size_t i = 0; i < localSubscriptions.size(); ++i)
    {
        file << "    {\"id\": \"" << escapeJson(localSubscriptions[i].id)
             << "\", \"name\": \"" << escapeJson(localSubscriptions[i].name)
             << "\", \"expiresAtDay\": " << localSubscriptions[i].expiresAtDay
             << ", \"cancellationRequested\": " << (localSubscriptions[i].cancellationRequested ? "true" : "false")
             << ", \"renewalPrice\": " << localSubscriptions[i].renewalPrice << "}";
        if (i + 1 < localSubscriptions.size())
        {
            file << ",";
        }
        file << "\n";
    }
    file << "  ],\n";

    file << "  \"activeBlessings\": [\n";
    const std::vector<Blessing>& activeBlessings = player.getActiveBlessings();
    for (std::size_t i = 0; i < activeBlessings.size(); ++i)
    {
        const Blessing& blessing = activeBlessings[i];
        file << "    {\"id\": \"" << escapeJson(blessing.getId())
             << "\", \"name\": \"" << escapeJson(blessing.getName())
             << "\", \"description\": \"" << escapeJson(blessing.getDescription())
             << "\", \"survivalProtection\": " << (blessing.grantsLethalSurvival() ? "true" : "false") << "}";
        if (i + 1 < activeBlessings.size())
        {
            file << ",";
        }
        file << "\n";
    }
    file << "  ],\n";

    file << "  \"activeCurses\": [\n";
    const std::vector<PlayerCurse>& activeCurses = player.getActiveCurses();
    for (std::size_t i = 0; i < activeCurses.size(); ++i)
    {
        const PlayerCurse& curse = activeCurses[i];
        file << "    {\"id\": \"" << escapeJson(curse.id)
             << "\", \"name\": \"" << escapeJson(curse.name)
             << "\", \"severity\": \"" << escapeJson(curse.severity)
             << "\", \"origin\": \"" << escapeJson(curse.origin)
             << "\", \"description\": \"" << escapeJson(curse.description)
             << "\", \"removalHint\": \"" << escapeJson(curse.removalHint)
             << "\", \"symptomCategories\": \"" << escapeJson(curse.symptomCategories)
             << "\", \"discoveredSymptomCategories\": \"" << escapeJson(curse.discoveredSymptomCategories)
             << "\", \"excludedSymptomCategories\": \"" << escapeJson(curse.excludedSymptomCategories)
             << "\", \"diagnosisLevel\": " << curse.diagnosisLevel
             << ", \"appliedAtDay\": " << curse.appliedAtDay
             << ", \"expiresAtDay\": " << curse.expiresAtDay
             << ", \"exorcismProgress\": " << curse.exorcismProgress
             << ", \"exorcismRequiredVisits\": " << curse.exorcismRequiredVisits
             << ", \"curseLevel\": " << curse.curseLevel
             << ", \"maxCurseLevel\": " << curse.maxCurseLevel
             << ", \"evolvesOverTime\": " << (curse.evolvesOverTime ? "true" : "false")
             << ", \"escalationIntervalDays\": " << curse.escalationIntervalDays
             << ", \"nextEscalationDay\": " << curse.nextEscalationDay
             << ", \"churchRemovalMaxLevel\": " << curse.churchRemovalMaxLevel
             << ", \"becomesSpecialRemovalWhenTooHigh\": " << (curse.becomesSpecialRemovalWhenTooHigh ? "true" : "false")
             << ", \"highLevelRemovalHint\": \"" << escapeJson(curse.highLevelRemovalHint)
             << "\", \"removableByChurch\": " << (curse.removableByChurch ? "true" : "false")
             << ", \"bossIdRequiredToBreak\": " << curse.bossIdRequiredToBreak
             << ", \"lifeLong\": " << (curse.lifeLong ? "true" : "false") << "}";
        if (i + 1 < activeCurses.size())
        {
            file << ",";
        }
        file << "\n";
    }
    file << "  ],\n";

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

    file << "  \"starterKitSnapshot\": [\n";
    const std::vector<std::string>& starterKitLog = player.getStarterKitLog();

    for (std::size_t i = 0; i < starterKitLog.size(); i++)
    {
        file << "    {\"label\": \"" << escapeJson(starterKitLog[i]) << "\"}";

        if (i + 1 < starterKitLog.size())
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
    file << "    \"enabledPassiveSkills\": [";
    const std::vector<std::string>& enabledPassives = player.getEnabledPassiveSkills();
    for (std::size_t i = 0; i < enabledPassives.size(); ++i)
    {
        file << "{\"id\": \"" << escapeJson(enabledPassives[i]) << "\"}";
        if (i + 1 < enabledPassives.size()) file << ", ";
    }
    file << "],\n";
    file << "    \"equippedActiveSkills\": [";
    const std::vector<std::string>& equippedActives = player.getEquippedActiveSkills();
    for (std::size_t i = 0; i < equippedActives.size(); ++i)
    {
        file << "{\"id\": \"" << escapeJson(equippedActives[i]) << "\"}";
        if (i + 1 < equippedActives.size()) file << ", ";
    }
    file << "],\n";
    file << "    \"daggerKillProgress\": " << player.getDaggerKillProgress() << ",\n";
    file << "    \"bowKillProgress\": " << player.getBowKillProgress() << ",\n";
    file << "    \"bareHandKillProgress\": " << player.getBareHandKillProgress() << ",\n";
    file << "    \"staffKillProgress\": " << player.getStaffKillProgress() << ",\n";
    file << "    \"swordKillProgress\": " << player.getSwordKillProgress() << ",\n";
    file << "    \"axeKillProgress\": " << player.getAxeKillProgress() << ",\n";
    file << "    \"hammerKillProgress\": " << player.getHammerKillProgress() << ",\n";
    file << "    \"spearKillProgress\": " << player.getSpearKillProgress() << "\n";
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
    file << "    \"recentBossCooldownExpiresAtDay\": " << player.getRecentBossCooldownExpiresAtDay() << ",\n";
    file << "    \"rareBossDiscoveryCooldownExpiresAtDay\": " << player.getRareBossDiscoveryCooldownExpiresAtDay() << ",\n";
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
    file << "],\n";
    file << "    \"defeatedBossIds\": [";
    const std::vector<int>& defeatedBossIds = player.getDefeatedBossIds();
    for (std::size_t i = 0; i < defeatedBossIds.size(); ++i)
    {
        file << "{\"id\":" << defeatedBossIds[i] << "}";
        if (i + 1 < defeatedBossIds.size())
        {
            file << ", ";
        }
    }
    file << "],\n";
    file << "    \"bossDiscoveryLocations\": [";
    const std::vector<std::string>& bossDiscoveryLocations = player.getBossDiscoveryLocations();
    bool wroteBossDiscoveryLocation = false;
    for (std::size_t id = 1; id < bossDiscoveryLocations.size(); ++id)
    {
        if (bossDiscoveryLocations[id].empty())
        {
            continue;
        }
        if (wroteBossDiscoveryLocation)
        {
            file << ", ";
        }
        file << "{\"id\":" << id << ",\"location\":\""
             << escapeJson(bossDiscoveryLocations[id]) << "\"}";
        wroteBossDiscoveryLocation = true;
    }
    file << "]\n";
    file << "  },\n";

    file << "  \"explorationVariety\": {\n";
    file << "    \"recentEventKeys\": [";
    const std::vector<std::string>& recentEventKeys = player.getRecentExplorationEventKeys();
    for (std::size_t i = 0; i < recentEventKeys.size(); ++i)
    {
        file << "{\"id\":\"" << escapeJson(recentEventKeys[i]) << "\"}";
        if (i + 1 < recentEventKeys.size()) file << ", ";
    }
    file << "],\n";
    file << "    \"recentChallengeKeys\": [";
    const std::vector<std::string>& recentChallengeKeys = player.getRecentExplorationChallengeKeys();
    for (std::size_t i = 0; i < recentChallengeKeys.size(); ++i)
    {
        file << "{\"id\":\"" << escapeJson(recentChallengeKeys[i]) << "\"}";
        if (i + 1 < recentChallengeKeys.size()) file << ", ";
    }
    file << "],\n";
    file << "    \"sceneCooldowns\": [";
    const std::vector<PlayerExplorationCooldown>& explorationCooldowns = player.getExplorationSceneCooldowns();
    bool wroteExplorationCooldown = false;
    for (const PlayerExplorationCooldown& cooldown : explorationCooldowns)
    {
        if (cooldown.key.empty() || cooldown.expiresAtDay <= player.getWorldDaysElapsed()) continue;
        if (wroteExplorationCooldown) file << ", ";
        file << "{\"key\":\"" << escapeJson(cooldown.key)
             << "\",\"expiresAtDay\":" << cooldown.expiresAtDay << "}";
        wroteExplorationCooldown = true;
    }
    file << "]\n";
    file << "  },\n";

    file << "  \"shopOfferState\": {\n";
    file << "    \"promotionPurchases\": [";
    const std::vector<PlayerPersistentCounter>& promotionCounters = player.getShopPromotionPurchaseCounters();
    for (std::size_t i = 0; i < promotionCounters.size(); ++i)
    {
        file << "{\"key\":\"" << escapeJson(promotionCounters[i].key)
             << "\",\"value\":" << promotionCounters[i].value << "}";
        if (i + 1 < promotionCounters.size()) file << ", ";
    }
    file << "]\n";
    file << "  },\n";

    file << "  \"canonicalJournal\": {\n";
    file << "    \"canonicalJournalSnapshot\": [";
    const std::vector<PlayerJournalRecord>& journalRecords = player.getCanonicalJournalRecords();
    for (std::size_t i = 0; i < journalRecords.size(); ++i)
    {
        file << "{\"category\":\"" << escapeJson(journalRecords[i].category)
             << "\",\"key\":\"" << escapeJson(journalRecords[i].key)
             << "\",\"label\":\"" << escapeJson(journalRecords[i].label)
             << "\",\"locationId\":\"" << escapeJson(journalRecords[i].locationId)
             << "\",\"count\":" << journalRecords[i].count
             << ",\"lastDay\":" << journalRecords[i].lastDay << "}";
        if (i + 1 < journalRecords.size()) file << ", ";
    }
    file << "]\n";
    file << "  },\n";

    file << "  \"worldMemory\": {\n";
    file << "    \"historicalEvents\": [";
    const std::vector<PlayerHistoricalEvent>& historicalEvents = player.getHistoricalEvents();
    for (std::size_t i = 0; i < historicalEvents.size(); ++i)
    {
        const PlayerHistoricalEvent& event = historicalEvents[i];
        file << "{\"id\":\"" << escapeJson(event.id)
             << "\",\"category\":\"" << escapeJson(event.category)
             << "\",\"subjectId\":\"" << escapeJson(event.subjectId)
             << "\",\"label\":\"" << escapeJson(event.label)
             << "\",\"locationId\":\"" << escapeJson(event.locationId)
             << "\",\"day\":" << event.day
             << ",\"resolved\":" << (event.resolved ? "true" : "false") << "}";
        if (i + 1 < historicalEvents.size()) file << ", ";
    }
    file << "],\n";
    file << "    \"rivals\": [";
    const std::vector<PlayerRivalRecord>& rivals = player.getRivalRecords();
    for (std::size_t i = 0; i < rivals.size(); ++i)
    {
        const PlayerRivalRecord& rival = rivals[i];
        file << "{\"rivalId\":\"" << escapeJson(rival.rivalId)
             << "\",\"enemyName\":\"" << escapeJson(rival.enemyName)
             << "\",\"enemyFamily\":\"" << escapeJson(rival.enemyFamily)
             << "\",\"originLocationId\":\"" << escapeJson(rival.originLocationId)
             << "\",\"lastKnownLocationId\":\"" << escapeJson(rival.lastKnownLocationId)
             << "\",\"rivalryReason\":\"" << escapeJson(rival.rivalryReason)
             << "\",\"temperament\":\"" << escapeJson(rival.temperament)
             << "\",\"visibleMark\":\"" << escapeJson(rival.visibleMark)
             << "\",\"lastOutcome\":\"" << escapeJson(rival.lastOutcome)
             << "\",\"baseLevel\":" << rival.baseLevel
             << ",\"currentLevel\":" << rival.currentLevel
             << ",\"baseMaxHp\":" << rival.baseMaxHp
             << ",\"baseAttack\":" << rival.baseAttack
             << ",\"encounters\":" << rival.encounters
             << ",\"escapes\":" << rival.escapes
             << ",\"returns\":" << rival.returns
             << ",\"wounds\":" << rival.wounds
             << ",\"emergenceScore\":" << rival.emergenceScore
             << ",\"notoriety\":" << rival.notoriety
             << ",\"firstSeenDay\":" << rival.firstSeenDay
             << ",\"lastSeenDay\":" << rival.lastSeenDay
             << ",\"alive\":" << (rival.alive ? "true" : "false")
             << ",\"stolenCopper\":" << rival.stolenCopper
             << ",\"stolenMaterialId\":\"" << escapeJson(rival.stolenMaterialId)
             << "\",\"stolenMaterialName\":\"" << escapeJson(rival.stolenMaterialName)
             << "\",\"stolenMaterialQuality\":\"" << escapeJson(rival.stolenMaterialQuality)
             << "\",\"stolenMaterialQuantity\":" << rival.stolenMaterialQuantity << "}";
        if (i + 1 < rivals.size()) file << ", ";
    }
    file << "]\n";
    file << "  },\n";

    file << "  \"npcMemoryState\": {\n";
    file << "    \"npcKnownFacts\": [";
    const std::vector<NpcKnownFact>& npcKnownFacts = player.getNpcKnownFacts();
    for (std::size_t i = 0; i < npcKnownFacts.size(); ++i)
    {
        const NpcKnownFact& fact = npcKnownFacts[i];
        file << "{\"npcId\":\"" << escapeJson(fact.npcId)
             << "\",\"factType\":\"" << escapeJson(fact.factType)
             << "\",\"subjectId\":\"" << escapeJson(fact.subjectId)
             << "\",\"label\":\"" << escapeJson(fact.label)
             << "\",\"sourceType\":\"" << escapeJson(fact.sourceType)
             << "\",\"sourceId\":\"" << escapeJson(fact.sourceId)
             << "\",\"locationId\":\"" << escapeJson(fact.locationId)
             << "\",\"claimVariant\":\"" << escapeJson(fact.claimVariant)
             << "\",\"relayChannel\":\"" << escapeJson(fact.relayChannel)
             << "\",\"firstLearnedDay\":" << fact.firstLearnedDay
             << ",\"lastReinforcedDay\":" << fact.lastReinforcedDay
             << ",\"confidence\":" << fact.confidence
             << ",\"evidenceLevel\":" << fact.evidenceLevel
             << ",\"timesHeard\":" << fact.timesHeard << "}";
        if (i + 1 < npcKnownFacts.size()) file << ", ";
    }
    file << "]\n";
    file << "  },\n";

    file << "  \"languageState\": {\n";
    file << "    \"languageKnowledge\": [";
    const std::vector<PlayerLanguageKnowledge>& languageKnowledge = player.getLanguageKnowledge();
    for (std::size_t i = 0; i < languageKnowledge.size(); ++i)
    {
        const PlayerLanguageKnowledge& language = languageKnowledge[i];
        file << "{\"id\":\"" << escapeJson(language.languageId)
             << "\",\"level\":" << language.level
             << ",\"studyProgress\":" << language.studyProgress
             << ",\"native\":" << (language.nativeLanguage ? "true" : "false") << "}";
        if (i + 1 < languageKnowledge.size()) file << ", ";
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
        file << "{\"persistentId\": \"" << escapeJson(stolenWeapon.getPersistentId()) << "\", \"name\": \"" << escapeJson(stolenWeapon.getName())
             << "\", \"description\": \"" << escapeJson(stolenWeapon.getDescription())
             << "\", \"value\": " << stolenWeapon.getValue()
             << ", \"type\": \"" << weaponTypeToSaveText(stolenWeapon.getType())
             << "\", \"minDamageBonus\": " << stolenWeapon.getMinDamageBonus()
             << ", \"maxDamageBonus\": " << stolenWeapon.getMaxDamageBonus()
             << ", \"criticalBonus\": " << stolenWeapon.getCriticalBonus()
             << ", \"durability\": " << stolenWeapon.getDurability()
             << ", \"maxDurability\": " << stolenWeapon.getMaxDurability()
             << ", \"enchantments\": \"" << escapeJson(stolenWeapon.getEnchantmentsSaveText()) << "\", \"ownershipBinding\": \"" << stolenWeapon.getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(stolenWeapon) << "}";
    }
    file << "],\n";
    file << "    \"grinkaStolenArmors\": [";
    if (player.hasGrinkaStolenArmor())
    {
        Armor stolenArmor = player.getGrinkaStolenArmor();
        file << "{\"persistentId\": \"" << escapeJson(stolenArmor.getPersistentId()) << "\", \"name\": \"" << escapeJson(stolenArmor.getName())
             << "\", \"description\": \"" << escapeJson(stolenArmor.getDescription())
             << "\", \"value\": " << stolenArmor.getValue()
             << ", \"type\": \"" << armorTypeToSaveText(stolenArmor.getType())
             << "\", \"maxHpBonus\": " << stolenArmor.getMaxHpBonus()
             << ", \"damageReduction\": " << stolenArmor.getDamageReduction()
             << ", \"durability\": " << stolenArmor.getDurability()
             << ", \"maxDurability\": " << stolenArmor.getMaxDurability()
             << ", \"enchantments\": \"" << escapeJson(stolenArmor.getEnchantmentsSaveText()) << "\", \"ownershipBinding\": \"" << stolenArmor.getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(stolenArmor) << "}";
    }
    file << "]\n";
    file << "  },\n";

    file << "  \"inventorySnapshot\": {\n";
    file << "    \"note\": \"Readable snapshot now reloaded for weapons, armors, consumables and materials.\",\n";
    file << "    \"weapons\": [\n";

    const std::vector<Weapon>& weapons = player.getInventory().getWeapons();

    for (std::size_t i = 0; i < weapons.size(); i++)
    {
        file << "      {\"persistentId\": \"" << escapeJson(weapons[i].getPersistentId()) << "\", \"name\": \"" << escapeJson(weapons[i].getName())
             << "\", \"description\": \"" << escapeJson(weapons[i].getDescription())
             << "\", \"value\": " << weapons[i].getValue()
             << ", \"type\": \"" << weaponTypeToSaveText(weapons[i].getType())
             << "\", \"minDamageBonus\": " << weapons[i].getMinDamageBonus()
             << ", \"maxDamageBonus\": " << weapons[i].getMaxDamageBonus()
             << ", \"criticalBonus\": " << weapons[i].getCriticalBonus()
             << ", \"durability\": " << weapons[i].getDurability()
             << ", \"maxDurability\": " << weapons[i].getMaxDurability()
             << ", \"enchantments\": \"" << escapeJson(weapons[i].getEnchantmentsSaveText()) << "\", \"ownershipBinding\": \"" << weapons[i].getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(weapons[i]) << "}";

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
        file << "      {\"persistentId\": \"" << escapeJson(armors[i].getPersistentId()) << "\", \"name\": \"" << escapeJson(armors[i].getName())
             << "\", \"description\": \"" << escapeJson(armors[i].getDescription())
             << "\", \"value\": " << armors[i].getValue()
             << ", \"type\": \"" << armorTypeToSaveText(armors[i].getType())
             << "\", \"maxHpBonus\": " << armors[i].getMaxHpBonus()
             << ", \"damageReduction\": " << armors[i].getDamageReduction()
             << ", \"durability\": " << armors[i].getDurability()
             << ", \"maxDurability\": " << armors[i].getMaxDurability()
             << ", \"enchantments\": \"" << escapeJson(armors[i].getEnchantmentsSaveText()) << "\", \"ownershipBinding\": \"" << armors[i].getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(armors[i]) << "}";

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
             << consumables[i].getPower() << ", \"ownershipBinding\": \""
             << consumables[i].getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(consumables[i]) << "}";

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
             << materials[i].getQuantity() << ", \"ownershipBinding\": \""
             << materials[i].getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(materials[i]) << "}";

        if (i + 1 < materials.size())
        {
            file << ",";
        }

        file << "\n";
    }

    file << "    ]\n";
    file << "  },\n";

    file << "  \"cityState\": {\n";
    file << "    \"currentCityId\": \"" << escapeJson(player.getCurrentCityId()) << "\",\n";
    file << "    \"registeredGuildCityIds\": [";
    const std::vector<std::string>& registeredGuildCityIds = player.getRegisteredGuildCityIds();
    for (std::size_t i = 0; i < registeredGuildCityIds.size(); ++i)
    {
        file << "{\"id\":\"" << escapeJson(registeredGuildCityIds[i]) << "\"}";
        if (i + 1 < registeredGuildCityIds.size()) file << ", ";
    }
    file << "],\n";
    file << "    \"vaultPurchased\": " << (player.hasCityVault() ? "true" : "false") << ",\n";
    file << "    \"vaultLevel\": " << player.getCityVaultLevel() << ",\n";
    file << "    \"vaultCapacity\": " << player.getCityVaultCapacity() << ",\n";
    file << "    \"vaultUsedSlots\": " << player.getCityVaultUsedSlots() << ",\n";
    file << "    \"vaultPlaceOptimized\": " << (player.hasCityVaultPlaceOptimization() ? "true" : "false") << ",\n";
    file << "    \"vaultPreparedStorage\": " << (player.hasCityVaultPreparedStorage() ? "true" : "false") << ",\n";
    file << "    \"backpackTier\": " << player.getBackpackTier() << ",\n";
    file << "    \"vaultRecallMessengerStamp\": " << player.getCityVaultLastRecallMessengerStamp() << ",\n";
    file << "    \"vaultRecallMessengerSourceCityId\": \"" << escapeJson(player.getCityVaultLastRecallMessengerSourceCityId()) << "\",\n";
    file << "    \"vaultRecallMessengerDestinationCityId\": \"" << escapeJson(player.getCityVaultLastRecallMessengerDestinationCityId()) << "\",\n";
    file << "    \"vaultWeapons\": [\n";
    const std::vector<Weapon>& vaultWeapons = player.getCityVault().getWeapons();
    for (std::size_t i = 0; i < vaultWeapons.size(); ++i)
    {
        file << "      {\"persistentId\": \"" << escapeJson(vaultWeapons[i].getPersistentId()) << "\", \"name\": \"" << escapeJson(vaultWeapons[i].getName())
             << "\", \"description\": \"" << escapeJson(vaultWeapons[i].getDescription())
             << "\", \"value\": " << vaultWeapons[i].getValue()
             << ", \"type\": \"" << weaponTypeToSaveText(vaultWeapons[i].getType())
             << "\", \"minDamageBonus\": " << vaultWeapons[i].getMinDamageBonus()
             << ", \"maxDamageBonus\": " << vaultWeapons[i].getMaxDamageBonus()
             << ", \"criticalBonus\": " << vaultWeapons[i].getCriticalBonus()
             << ", \"durability\": " << vaultWeapons[i].getDurability()
             << ", \"maxDurability\": " << vaultWeapons[i].getMaxDurability()
             << ", \"enchantments\": \"" << escapeJson(vaultWeapons[i].getEnchantmentsSaveText()) << "\", \"ownershipBinding\": \"" << vaultWeapons[i].getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(vaultWeapons[i]) << "}";
        if (i + 1 < vaultWeapons.size()) file << ",";
        file << "\n";
    }
    file << "    ],\n";
    file << "    \"vaultArmors\": [\n";
    const std::vector<Armor>& vaultArmors = player.getCityVault().getArmors();
    for (std::size_t i = 0; i < vaultArmors.size(); ++i)
    {
        file << "      {\"persistentId\": \"" << escapeJson(vaultArmors[i].getPersistentId()) << "\", \"name\": \"" << escapeJson(vaultArmors[i].getName())
             << "\", \"description\": \"" << escapeJson(vaultArmors[i].getDescription())
             << "\", \"value\": " << vaultArmors[i].getValue()
             << ", \"type\": \"" << armorTypeToSaveText(vaultArmors[i].getType())
             << "\", \"maxHpBonus\": " << vaultArmors[i].getMaxHpBonus()
             << ", \"damageReduction\": " << vaultArmors[i].getDamageReduction()
             << ", \"durability\": " << vaultArmors[i].getDurability()
             << ", \"maxDurability\": " << vaultArmors[i].getMaxDurability()
             << ", \"enchantments\": \"" << escapeJson(vaultArmors[i].getEnchantmentsSaveText()) << "\", \"ownershipBinding\": \"" << vaultArmors[i].getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(vaultArmors[i]) << "}";
        if (i + 1 < vaultArmors.size()) file << ",";
        file << "\n";
    }
    file << "    ],\n";
    file << "    \"vaultConsumables\": [\n";
    const std::vector<Consumable>& vaultConsumables = player.getCityVault().getConsumables();
    for (std::size_t i = 0; i < vaultConsumables.size(); ++i)
    {
        file << "      {\"name\": \"" << escapeJson(vaultConsumables[i].getName())
             << "\", \"power\": " << vaultConsumables[i].getPower()
             << ", \"ownershipBinding\": \"" << vaultConsumables[i].getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(vaultConsumables[i]) << "}";
        if (i + 1 < vaultConsumables.size()) file << ",";
        file << "\n";
    }
    file << "    ],\n";
    file << "    \"vaultMaterials\": [\n";
    const std::vector<Material>& vaultMaterials = player.getCityVault().getMaterials();
    for (std::size_t i = 0; i < vaultMaterials.size(); ++i)
    {
        file << "      {\"id\": \"" << escapeJson(vaultMaterials[i].getId())
             << "\", \"name\": \"" << escapeJson(vaultMaterials[i].getName())
             << "\", \"category\": \"" << escapeJson(vaultMaterials[i].getCategory())
             << "\", \"quality\": \"" << escapeJson(vaultMaterials[i].getQuality())
             << "\", \"quantity\": " << vaultMaterials[i].getQuantity()
             << ", \"ownershipBinding\": \"" << vaultMaterials[i].getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(vaultMaterials[i]) << "}";
        if (i + 1 < vaultMaterials.size()) file << ",";
        file << "\n";
    }
    file << "    ],\n";

    std::vector<PlayerCityVault> allVaultRecords = player.getCityVaultRecords();
    bool currentVaultAlreadyListed = false;
    for (const PlayerCityVault& record : allVaultRecords)
    {
        if (record.cityId == player.getCurrentCityId())
        {
            currentVaultAlreadyListed = true;
            break;
        }
    }
    if (!currentVaultAlreadyListed && player.hasCityVault())
    {
        PlayerCityVault currentRecord;
        currentRecord.cityId = player.getCurrentCityId();
        currentRecord.inventory = player.getCityVault();
        currentRecord.purchased = player.hasCityVault();
        currentRecord.level = player.getCityVaultLevel();
        currentRecord.placeOptimized = player.hasCityVaultPlaceOptimization();
        currentRecord.preparedStorage = player.hasCityVaultPreparedStorage();
        currentRecord.securityDaysRemaining = player.getCityVaultSecurityDaysRemainingForCity(player.getCurrentCityId());
        currentRecord.raidPending = player.hasCityVaultRaidPendingForCity(player.getCurrentCityId());
        currentRecord.raidDeadlineDay = player.getCityVaultRaidDeadlineDayForCity(player.getCurrentCityId());
        allVaultRecords.push_back(currentRecord);
    }

    file << "    \"cityVaults\": [";
    for (std::size_t i = 0; i < allVaultRecords.size(); ++i)
    {
        const PlayerCityVault& record = allVaultRecords[i];
        file << "{\"cityId\":\"" << escapeJson(record.cityId)
             << "\",\"purchased\":" << (record.purchased ? "true" : "false")
             << ",\"level\":" << record.level
             << ",\"capacity\":" << player.getCityVaultCapacityForCity(record.cityId)
             << ",\"usedSlots\":" << player.getCityVaultUsedSlotsForCity(record.cityId)
             << ",\"placeOptimized\":" << (record.placeOptimized ? "true" : "false")
             << ",\"preparedStorage\":" << (record.preparedStorage ? "true" : "false")
             << ",\"storageReductionPercent\":" << ((record.placeOptimized ? 10 : 0) + (record.preparedStorage ? 15 : 0))
             << ",\"securityDaysRemaining\":" << std::max(0, record.securityDaysRemaining)
             << ",\"raidPending\":" << (record.raidPending ? "true" : "false")
             << ",\"raidDeadlineDay\":" << (record.raidPending ? record.raidDeadlineDay : -1)
             << ",\"lastRaidCheckDay\":" << record.lastRaidCheckDay
             << ",\"platinumCoins\":" << record.inventory.getCoinStacks().platinum
             << ",\"goldCoins\":" << record.inventory.getCoinStacks().gold
             << ",\"electrumCoins\":" << record.inventory.getCoinStacks().electrum
             << ",\"ironCoins\":" << record.inventory.getCoinStacks().iron
             << ",\"copperCoins\":" << record.inventory.getCoinStacks().copper
             << ",\"remoteWithdrawalAllowed\":false"
             << ",\"remoteViewOnly\":true}";
        if (i + 1 < allVaultRecords.size()) file << ", ";
    }
    file << "],\n";

    file << "    \"cityVaultWeapons\": [";
    bool wroteCityVaultWeapon = false;
    for (const PlayerCityVault& record : allVaultRecords)
    {
        const std::vector<Weapon>& recordWeapons = record.inventory.getWeapons();
        for (const Weapon& weaponEntry : recordWeapons)
        {
            if (wroteCityVaultWeapon) file << ", ";
            file << "{\"cityId\":\"" << escapeJson(record.cityId)
                 << "\",\"persistentId\":\"" << escapeJson(weaponEntry.getPersistentId())
                 << "\",\"name\":\"" << escapeJson(weaponEntry.getName())
                 << "\",\"description\":\"" << escapeJson(weaponEntry.getDescription())
                 << "\",\"value\":" << weaponEntry.getValue()
                 << ",\"type\":\"" << weaponTypeToSaveText(weaponEntry.getType())
                 << "\",\"minDamageBonus\":" << weaponEntry.getMinDamageBonus()
                 << ",\"maxDamageBonus\":" << weaponEntry.getMaxDamageBonus()
                 << ",\"criticalBonus\":" << weaponEntry.getCriticalBonus()
                 << ",\"durability\":" << weaponEntry.getDurability()
                 << ",\"maxDurability\":" << weaponEntry.getMaxDurability()
                 << ",\"enchantments\":\"" << escapeJson(weaponEntry.getEnchantmentsSaveText()) << "\",\"ownershipBinding\":\"" << weaponEntry.getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(weaponEntry) << "}";
            wroteCityVaultWeapon = true;
        }
    }
    file << "],\n";

    file << "    \"cityVaultArmors\": [";
    bool wroteCityVaultArmor = false;
    for (const PlayerCityVault& record : allVaultRecords)
    {
        const std::vector<Armor>& recordArmors = record.inventory.getArmors();
        for (const Armor& armorEntry : recordArmors)
        {
            if (wroteCityVaultArmor) file << ", ";
            file << "{\"cityId\":\"" << escapeJson(record.cityId)
                 << "\",\"persistentId\":\"" << escapeJson(armorEntry.getPersistentId())
                 << "\",\"name\":\"" << escapeJson(armorEntry.getName())
                 << "\",\"description\":\"" << escapeJson(armorEntry.getDescription())
                 << "\",\"value\":" << armorEntry.getValue()
                 << ",\"type\":\"" << armorTypeToSaveText(armorEntry.getType())
                 << "\",\"maxHpBonus\":" << armorEntry.getMaxHpBonus()
                 << ",\"damageReduction\":" << armorEntry.getDamageReduction()
                 << ",\"durability\":" << armorEntry.getDurability()
                 << ",\"maxDurability\":" << armorEntry.getMaxDurability()
                 << ",\"enchantments\":\"" << escapeJson(armorEntry.getEnchantmentsSaveText()) << "\",\"ownershipBinding\":\"" << armorEntry.getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(armorEntry) << "}";
            wroteCityVaultArmor = true;
        }
    }
    file << "],\n";

    file << "    \"cityVaultConsumables\": [";
    bool wroteCityVaultConsumable = false;
    for (const PlayerCityVault& record : allVaultRecords)
    {
        const std::vector<Consumable>& recordConsumables = record.inventory.getConsumables();
        for (const Consumable& consumableEntry : recordConsumables)
        {
            if (wroteCityVaultConsumable) file << ", ";
            file << "{\"cityId\":\"" << escapeJson(record.cityId)
                 << "\",\"name\":\"" << escapeJson(consumableEntry.getName())
                 << "\",\"power\":" << consumableEntry.getPower()
                 << ",\"ownershipBinding\":\"" << consumableEntry.getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(consumableEntry) << "}";
            wroteCityVaultConsumable = true;
        }
    }
    file << "],\n";

    file << "    \"cityVaultMaterials\": [";
    bool wroteCityVaultMaterial = false;
    for (const PlayerCityVault& record : allVaultRecords)
    {
        const std::vector<Material>& recordMaterials = record.inventory.getMaterials();
        for (const Material& materialEntry : recordMaterials)
        {
            if (wroteCityVaultMaterial) file << ", ";
            file << "{\"cityId\":\"" << escapeJson(record.cityId)
                 << "\",\"id\":\"" << escapeJson(materialEntry.getId())
                 << "\",\"name\":\"" << escapeJson(materialEntry.getName())
                 << "\",\"category\":\"" << escapeJson(materialEntry.getCategory())
                 << "\",\"quality\":\"" << escapeJson(materialEntry.getQuality())
                 << "\",\"quantity\":" << materialEntry.getQuantity()
                 << ",\"ownershipBinding\":\"" << materialEntry.getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(materialEntry) << "}";
            wroteCityVaultMaterial = true;
        }
    }
    file << "],\n";

    file << "    \"battlefieldRecoveryCaches\": [";
    const auto& battlefieldCaches = player.getBattlefieldRecoveryCaches();
    for (std::size_t i = 0; i < battlefieldCaches.size(); ++i)
    {
        const PlayerBattlefieldRecoveryCache& cache = battlefieldCaches[i];
        const CoinBreakdown coins = cache.inventory.getCoinStacks();
        file << "{\"cacheId\":\"" << escapeJson(cache.cacheId)
             << "\",\"cityId\":\"" << escapeJson(cache.cityId)
             << "\",\"contextId\":\"" << escapeJson(cache.contextId)
             << "\",\"locationLabel\":\"" << escapeJson(cache.locationLabel)
             << "\",\"createdDay\":" << cache.createdDay
             << ",\"lastAttemptDay\":" << cache.lastAttemptDay
             << ",\"searchAttempts\":" << cache.searchAttempts
             << ",\"disturbanceLevel\":" << cache.disturbanceLevel
             << ",\"custodyState\":\"" << escapeJson(cache.custodyState)
             << "\",\"custodyLabel\":\"" << escapeJson(cache.custodyLabel)
             << "\",\"lastMovementDay\":" << cache.lastMovementDay
             << ",\"movementCount\":" << cache.movementCount
             << ",\"locationKnown\":" << (cache.locationKnown ? "true" : "false")
             << ",\"resolved\":" << (cache.resolved ? "true" : "false")
             << ",\"platinumCoins\":" << coins.platinum
             << ",\"goldCoins\":" << coins.gold
             << ",\"electrumCoins\":" << coins.electrum
             << ",\"ironCoins\":" << coins.iron
             << ",\"copperCoins\":" << coins.copper << "}";
        if (i + 1 < battlefieldCaches.size()) file << ", ";
    }
    file << "],\n";

    file << "    \"battlefieldRecoveryWeapons\": [";
    bool wroteBattlefieldWeapon = false;
    for (const PlayerBattlefieldRecoveryCache& cache : battlefieldCaches)
    {
        for (const Weapon& w : cache.inventory.getWeapons())
        {
            if (wroteBattlefieldWeapon) file << ", ";
            file << "{\"cacheId\":\"" << escapeJson(cache.cacheId)
                 << "\",\"persistentId\":\"" << escapeJson(w.getPersistentId())
                 << "\",\"name\":\"" << escapeJson(w.getName())
                 << "\",\"description\":\"" << escapeJson(w.getDescription())
                 << "\",\"value\":" << w.getValue()
                 << ",\"type\":\"" << weaponTypeToSaveText(w.getType())
                 << "\",\"minDamageBonus\":" << w.getMinDamageBonus()
                 << ",\"maxDamageBonus\":" << w.getMaxDamageBonus()
                 << ",\"criticalBonus\":" << w.getCriticalBonus()
                 << ",\"durability\":" << w.getDurability()
                 << ",\"maxDurability\":" << w.getMaxDurability()
                 << ",\"enchantments\":\"" << escapeJson(w.getEnchantmentsSaveText())
                 << "\",\"ownershipBinding\":\"" << w.getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(w) << "}";
            wroteBattlefieldWeapon = true;
        }
    }
    file << "],\n";

    file << "    \"battlefieldRecoveryArmors\": [";
    bool wroteBattlefieldArmor = false;
    for (const PlayerBattlefieldRecoveryCache& cache : battlefieldCaches)
    {
        for (const Armor& a : cache.inventory.getArmors())
        {
            if (wroteBattlefieldArmor) file << ", ";
            file << "{\"cacheId\":\"" << escapeJson(cache.cacheId)
                 << "\",\"persistentId\":\"" << escapeJson(a.getPersistentId())
                 << "\",\"name\":\"" << escapeJson(a.getName())
                 << "\",\"description\":\"" << escapeJson(a.getDescription())
                 << "\",\"value\":" << a.getValue()
                 << ",\"type\":\"" << armorTypeToSaveText(a.getType())
                 << "\",\"maxHpBonus\":" << a.getMaxHpBonus()
                 << ",\"damageReduction\":" << a.getDamageReduction()
                 << ",\"durability\":" << a.getDurability()
                 << ",\"maxDurability\":" << a.getMaxDurability()
                 << ",\"enchantments\":\"" << escapeJson(a.getEnchantmentsSaveText())
                 << "\",\"ownershipBinding\":\"" << a.getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(a) << "}";
            wroteBattlefieldArmor = true;
        }
    }
    file << "],\n";

    file << "    \"battlefieldRecoveryConsumables\": [";
    bool wroteBattlefieldConsumable = false;
    for (const PlayerBattlefieldRecoveryCache& cache : battlefieldCaches)
    {
        for (const Consumable& c : cache.inventory.getConsumables())
        {
            if (wroteBattlefieldConsumable) file << ", ";
            file << "{\"cacheId\":\"" << escapeJson(cache.cacheId)
                 << "\",\"name\":\"" << escapeJson(c.getName())
                 << "\",\"power\":" << c.getPower()
                 << ",\"ownershipBinding\":\"" << c.getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(c) << "}";
            wroteBattlefieldConsumable = true;
        }
    }
    file << "],\n";

    file << "    \"battlefieldRecoveryMaterials\": [";
    bool wroteBattlefieldMaterial = false;
    for (const PlayerBattlefieldRecoveryCache& cache : battlefieldCaches)
    {
        for (const Material& m : cache.inventory.getMaterials())
        {
            if (wroteBattlefieldMaterial) file << ", ";
            file << "{\"cacheId\":\"" << escapeJson(cache.cacheId)
                 << "\",\"id\":\"" << escapeJson(m.getId())
                 << "\",\"name\":\"" << escapeJson(m.getName())
                 << "\",\"category\":\"" << escapeJson(m.getCategory())
                 << "\",\"quality\":\"" << escapeJson(m.getQuality())
                 << "\",\"quantity\":" << m.getQuantity()
                 << ",\"ownershipBinding\":\"" << m.getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(m) << "}";
            wroteBattlefieldMaterial = true;
        }
    }
    file << "],\n";

    file << "    \"crimeContactStates\": [";
    const auto& crimeContacts = player.getCrimeContactStates();
    for (std::size_t i = 0; i < crimeContacts.size(); ++i)
    {
        const PlayerCrimeContactState& c = crimeContacts[i];
        file << "{\"contactId\":\"" << escapeJson(c.contactId)
             << "\",\"displayName\":\"" << escapeJson(c.displayName)
             << "\",\"firstSeenDay\":" << c.firstSeenDay
             << ",\"lastInteractionDay\":" << c.lastInteractionDay
             << ",\"encounters\":" << c.encounters
             << ",\"observations\":" << c.observations
             << ",\"negotiations\":" << c.negotiations
             << ",\"guardConfrontations\":" << c.guardConfrontations
             << ",\"pressure\":" << c.pressure
             << ",\"cooperation\":" << c.cooperation
             << ",\"fledUntilDay\":" << c.fledUntilDay
             << ",\"detainedUntilDay\":" << c.detainedUntilDay
             << ",\"provisionalDetentions\":" << c.provisionalDetentions
             << ",\"releases\":" << c.releases
             << ",\"returnsAfterFlight\":" << c.returnsAfterFlight
             << ",\"lastReleaseDay\":" << c.lastReleaseDay
             << ",\"lastReturnDay\":" << c.lastReturnDay
             << ",\"knownRole\":\"" << escapeJson(c.knownRole) << "\""
             << ",\"homeCityId\":\"" << escapeJson(c.homeCityId) << "\""
             << ",\"networkId\":\"" << escapeJson(c.networkId) << "\""
             << ",\"networkWarnings\":" << c.networkWarnings
             << ",\"lastNetworkWarningDay\":" << c.lastNetworkWarningDay
             << ",\"lastWarningSourceId\":\"" << escapeJson(c.lastWarningSourceId) << "\""
             << ",\"reliability\":" << c.reliability
             << ",\"favorBalance\":" << c.favorBalance
             << ",\"rivalry\":" << c.rivalry
             << ",\"fulfilledDeals\":" << c.fulfilledDeals
             << ",\"brokenDeals\":" << c.brokenDeals
             << ",\"ruptureUntilDay\":" << c.ruptureUntilDay
             << ",\"reportedToGuard\":" << (c.reportedToGuard ? "true" : "false")
             << ",\"arrested\":" << (c.arrested ? "true" : "false") << "}";
        if (i + 1 < crimeContacts.size()) file << ", ";
    }
    file << "],\n";

    file << "    \"vaultTheftCases\": [";
    const auto& theftCases = player.getVaultTheftCases();
    for (std::size_t i = 0; i < theftCases.size(); ++i)
    {
        const PlayerVaultTheftCase& c = theftCases[i];
        file << "{\"caseId\":\"" << escapeJson(c.caseId)
             << "\",\"cityId\":\"" << escapeJson(c.cityId)
             << "\",\"createdDay\":" << c.createdDay
             << ",\"lastLeadDay\":" << c.lastLeadDay
             << ",\"clues\":" << c.clues
             << ",\"culpritIdentified\":" << (c.culpritIdentified ? "true" : "false")
             << ",\"identifiedRivalId\":\"" << escapeJson(c.identifiedRivalId)
             << "\",\"holderType\":\"" << escapeJson(c.holderType)
             << "\",\"holderLabel\":\"" << escapeJson(c.holderLabel)
             << "\",\"holderWorldContactId\":\"" << escapeJson(c.holderWorldContactId)
             << "\",\"holderLocationId\":\"" << escapeJson(c.holderLocationId)
             << "\",\"holderLocationLabel\":\"" << escapeJson(c.holderLocationLabel)
             << "\",\"holderExplorationBiome\":\"" << escapeJson(c.holderExplorationBiome)
             << "\",\"handoffCount\":" << c.handoffCount
             << ",\"holderObservationCount\":" << c.holderObservationCount
             << ",\"negotiationAttempts\":" << c.negotiationAttempts
             << ",\"confrontationAttempts\":" << c.confrontationAttempts
             << ",\"holderPressure\":" << c.holderPressure
             << ",\"holderCooperation\":" << c.holderCooperation
             << ",\"holderClosedUntilDay\":" << c.holderClosedUntilDay
             << ",\"holderReportedToGuard\":" << (c.holderReportedToGuard ? "true" : "false")
             << ",\"lastHolderInteractionDay\":" << c.lastHolderInteractionDay
             << ",\"holderAlerted\":" << (c.holderAlerted ? "true" : "false")
             << ",\"resolved\":" << (c.resolved ? "true" : "false")
             << ",\"stolenCopper\":" << c.stolenCopper
             << ",\"hasWeapon\":" << (c.hasWeapon ? "true" : "false")
             << ",\"hasArmor\":" << (c.hasArmor ? "true" : "false")
             << ",\"hasConsumable\":" << (c.hasConsumable ? "true" : "false")
             << ",\"stolenConsumableName\":\"" << escapeJson(c.hasConsumable ? c.stolenConsumable.getName() : "")
             << "\",\"stolenConsumableOwnershipBinding\":\"" << (c.hasConsumable ? c.stolenConsumable.getOwnershipBindingSaveText() : "none")
             << "\",\"stolenMaterialId\":\"" << escapeJson(c.stolenMaterialId)
             << "\",\"stolenMaterialName\":\"" << escapeJson(c.stolenMaterialName)
             << "\",\"stolenMaterialQuality\":\"" << escapeJson(c.stolenMaterialQuality)
             << "\",\"stolenMaterialQuantity\":" << c.stolenMaterialQuantity << "}";
        if (i + 1 < theftCases.size()) file << ", ";
    }
    file << "],\n";

    file << "    \"vaultTheftCaseWeapons\": [";
    bool wroteTheftWeapon = false;
    for (const PlayerVaultTheftCase& c : theftCases)
    {
        if (!c.hasWeapon) continue;
        const Weapon& w = c.stolenWeapon;
        if (wroteTheftWeapon) file << ", ";
        file << "{\"caseId\":\"" << escapeJson(c.caseId)
             << "\",\"persistentId\":\"" << escapeJson(w.getPersistentId())
             << "\",\"name\":\"" << escapeJson(w.getName())
             << "\",\"description\":\"" << escapeJson(w.getDescription())
             << "\",\"value\":" << w.getValue()
             << ",\"type\":\"" << weaponTypeToSaveText(w.getType())
             << "\",\"minDamageBonus\":" << w.getMinDamageBonus()
             << ",\"maxDamageBonus\":" << w.getMaxDamageBonus()
             << ",\"criticalBonus\":" << w.getCriticalBonus()
             << ",\"durability\":" << w.getDurability()
             << ",\"maxDurability\":" << w.getMaxDurability()
             << ",\"enchantments\":\"" << escapeJson(w.getEnchantmentsSaveText()) << "\",\"ownershipBinding\":\"" << w.getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(w) << "}";
        wroteTheftWeapon = true;
    }
    file << "],\n";

    file << "    \"vaultTheftCaseArmors\": [";
    bool wroteTheftArmor = false;
    for (const PlayerVaultTheftCase& c : theftCases)
    {
        if (!c.hasArmor) continue;
        const Armor& a = c.stolenArmor;
        if (wroteTheftArmor) file << ", ";
        file << "{\"caseId\":\"" << escapeJson(c.caseId)
             << "\",\"persistentId\":\"" << escapeJson(a.getPersistentId())
             << "\",\"name\":\"" << escapeJson(a.getName())
             << "\",\"description\":\"" << escapeJson(a.getDescription())
             << "\",\"value\":" << a.getValue()
             << ",\"type\":\"" << armorTypeToSaveText(a.getType())
             << "\",\"maxHpBonus\":" << a.getMaxHpBonus()
             << ",\"damageReduction\":" << a.getDamageReduction()
             << ",\"durability\":" << a.getDurability()
             << ",\"maxDurability\":" << a.getMaxDurability()
             << ",\"enchantments\":\"" << escapeJson(a.getEnchantmentsSaveText()) << "\",\"ownershipBinding\":\"" << a.getOwnershipBindingSaveText() << "\"" << recoveryMetadataJson(a) << "}";
        wroteTheftArmor = true;
    }
    file << "]\n";
    file << "  },\n";

    file << "  \"materialKnowledgeSnapshot\": [\n";
    const std::vector<MaterialKnowledgeRecord> materialKnowledgeRecords = MaterialKnowledgeProgress::getRecords();
    for (std::size_t i = 0; i < materialKnowledgeRecords.size(); ++i)
    {
        const MaterialKnowledgeRecord& record = materialKnowledgeRecords[i];
        file << "    {\"id\": \"" << escapeJson(record.id)
             << "\", \"name\": \"" << escapeJson(record.name)
             << "\", \"category\": \"" << escapeJson(record.category)
             << "\", \"quality\": \"" << escapeJson(record.quality)
             << "\", \"discoveredQuantity\": " << record.discoveredQuantity
             << ", \"bestQualityWeight\": " << record.bestQualityWeight << "}";
        if (i + 1 < materialKnowledgeRecords.size()) file << ",";
        file << "\n";
    }
    file << "  ],\n";

    file << "  \"bestiaryRuntimeSnapshot\": [\n";

    std::vector<BestiaryRuntimeRecord> bestiaryRecords = BestiaryRuntimeProgress::getRecords();

    for (std::size_t i = 0; i < bestiaryRecords.size(); i++)
    {
        const BestiaryRuntimeRecord& record = bestiaryRecords[i];

        file << "    {\"category\": \"" << escapeJson(record.category)
             << "\", \"name\": \"" << escapeJson(record.name)
             << "\", \"description\": \"" << escapeJson(record.description)
             << "\", \"status\": \"" << escapeJson(record.status)
             << "\", \"habitat\": \"" << escapeJson(record.habitat)
             << "\", \"weaknesses\": \"" << escapeJson(record.weaknesses)
             << "\", \"resistances\": \"" << escapeJson(record.resistances)
             << "\", \"drops\": \"" << escapeJson(record.drops)
             << "\", \"strategy\": \"" << escapeJson(record.strategy)
             << "\", \"dangerRank\": \"" << escapeJson(record.dangerRank)
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

    auto writeQuestSnapshotEntry = [&file](const Quest& quest, const std::string& indent) {
        file << indent << "{\"id\": \"" << escapeJson(quest.id)
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
             << ", \"rewardCoinCopper\": " << quest.rewardCoins.copper
             << ", \"rewardCoinIron\": " << quest.rewardCoins.iron
             << ", \"rewardCoinElectrum\": " << quest.rewardCoins.electrum
             << ", \"rewardCoinGold\": " << quest.rewardCoins.gold
             << ", \"rewardCoinPlatinum\": " << quest.rewardCoins.platinum
             << ", \"rewardMaterialId\": \"" << escapeJson(quest.rewardMaterialId)
             << "\", \"rewardMaterialName\": \"" << escapeJson(quest.rewardMaterialName)
             << "\", \"rewardMaterialQuantity\": " << quest.rewardMaterialQuantity
             << ", \"rewardNote\": \"" << escapeJson(quest.rewardNote)
             << "\""
             << ", \"requiredMaterialId\": \"" << escapeJson(quest.requiredMaterialId)
             << "\", \"requiredMaterialName\": \"" << escapeJson(quest.requiredMaterialName)
             << "\", \"requiredMaterialQuantity\": " << quest.requiredMaterialQuantity
             << ", \"progress\": " << quest.progress
             << ", \"target\": " << quest.target
             << ", \"availableFromDay\": " << quest.availableFromDay
             << ", \"expiresAtDay\": " << quest.expiresAtDay
             << ", \"guildQuest\": " << (quest.guildQuest ? "true" : "false")
             << ", \"guildChallenge\": " << (quest.guildChallenge ? "true" : "false")
             << ", \"challengeCondition\": \"" << escapeJson(quest.challengeCondition) << "\""
             << ", \"challengeMarkReward\": " << quest.challengeMarkReward
             << ", \"accepted\": " << (quest.accepted ? "true" : "false")
             << ", \"completed\": " << (quest.completed ? "true" : "false")
             << ", \"turnedIn\": " << (quest.turnedIn ? "true" : "false")
             << ", \"failed\": " << (quest.failed ? "true" : "false")
             << ", \"failureReason\": \"" << escapeJson(quest.failureReason) << "\""
             << ", \"requiredLanguage\": \"" << escapeJson(quest.requiredLanguage) << "\""
             << ", \"requiredLanguageLevel\": " << quest.requiredLanguageLevel
             << ", \"sourceLanguageText\": \"" << escapeJson(quest.sourceLanguageText) << "\""
             << ", \"linkedQuestIds\": \"" << escapeJson(quest.linkedQuestIds) << "\""
             << ", \"stageLabels\": \"" << escapeJson(quest.stageLabels) << "\""
             << ", \"serviceChallengeHistory\": \"" << escapeJson(quest.serviceChallengeHistory) << "\""
             << ", \"linkedQuestRequiredState\": \"" << escapeJson(quest.linkedQuestRequiredState) << "\""
             << ", \"retroactiveProgress\": " << (quest.retroactiveProgress ? "true" : "false")
             << ", \"hideFutureSteps\": " << (quest.hideFutureSteps ? "true" : "false")
             << ", \"phaseTwoChecked\": " << (quest.phaseTwoChecked ? "true" : "false")
             << ", \"phaseTwoOffered\": " << (quest.phaseTwoOffered ? "true" : "false")
             << ", \"phaseTwoActive\": " << (quest.phaseTwoActive ? "true" : "false")
             << ", \"phaseTwoMandatory\": " << (quest.phaseTwoMandatory ? "true" : "false")
             << ", \"phaseTwoCompleted\": " << (quest.phaseTwoCompleted ? "true" : "false")
             << ", \"phaseTwoDeclined\": " << (quest.phaseTwoDeclined ? "true" : "false")
             << ", \"phaseTwoTitle\": \"" << escapeJson(quest.phaseTwoTitle) << "\""
             << ", \"phaseTwoPreviousSummary\": \"" << escapeJson(quest.phaseTwoPreviousSummary) << "\""
             << ", \"phaseTwoReason\": \"" << escapeJson(quest.phaseTwoReason) << "\""
             << ", \"phaseTwoObjective\": \"" << escapeJson(quest.phaseTwoObjective) << "\""
             << ", \"phaseTwoProgress\": " << quest.phaseTwoProgress
             << ", \"phaseTwoTarget\": " << quest.phaseTwoTarget
             << ", \"phaseTwoAttempts\": " << quest.phaseTwoAttempts
             << ", \"phaseTwoRewardExperience\": " << quest.phaseTwoRewardExperience
             << ", \"phaseTwoRewardGold\": " << quest.phaseTwoRewardGold
             << ", \"phaseTwoRewardKind\": \"" << escapeJson(quest.phaseTwoRewardKind) << "\""
             << ", \"phaseTwoRewardDetail\": \"" << escapeJson(quest.phaseTwoRewardDetail) << "\""
             << ", \"phaseTwoTravelRequired\": " << (quest.phaseTwoTravelRequired ? "true" : "false")
             << ", \"phaseTwoTravelSourceCityId\": \"" << escapeJson(quest.phaseTwoTravelSourceCityId) << "\""
             << ", \"phaseTwoTravelDestinationCityId\": \"" << escapeJson(quest.phaseTwoTravelDestinationCityId) << "\""
             << ", \"phaseTwoTravelDistanceKm\": " << quest.phaseTwoTravelDistanceKm
             << ", \"intercityMission\": " << (quest.intercityMission ? "true" : "false")
             << ", \"intercityKind\": \"" << escapeJson(quest.intercityKind) << "\""
             << ", \"intercitySourceCityId\": \"" << escapeJson(quest.intercitySourceCityId) << "\""
             << ", \"intercityRouteCityIds\": \"" << escapeJson(quest.intercityRouteCityIds) << "\""
             << ", \"intercityPayloadName\": \"" << escapeJson(quest.intercityPayloadName) << "\""
             << ", \"intercityStopIndex\": " << quest.intercityStopIndex
             << ", \"intercityTotalDistanceKm\": " << quest.intercityTotalDistanceKm
             << ", \"intercityTravelRewardGold\": " << quest.intercityTravelRewardGold
             << ", \"intercityTravelRewardExperience\": " << quest.intercityTravelRewardExperience
             << ", \"tradeIncidentMission\": " << (quest.tradeIncidentMission ? "true" : "false")
             << ", \"tradeIncidentId\": \"" << escapeJson(quest.tradeIncidentId) << "\""
             << ", \"tradeIncidentKind\": \"" << escapeJson(quest.tradeIncidentKind) << "\""
             << ", \"tradeIncidentSourceCityId\": \"" << escapeJson(quest.tradeIncidentSourceCityId) << "\""
             << ", \"tradeIncidentDestinationCityId\": \"" << escapeJson(quest.tradeIncidentDestinationCityId) << "\""
             << ", \"tradeIncidentGoodsType\": " << quest.tradeIncidentGoodsType
             << ", \"tradeIncidentSeverity\": " << quest.tradeIncidentSeverity
             << ", \"tradeIncidentStage\": \"" << escapeJson(quest.tradeIncidentStage) << "\""
             << ", \"tradeIncidentAttempts\": " << quest.tradeIncidentAttempts
             << ", \"tradeIncidentCreatesBountyLead\": " << (quest.tradeIncidentCreatesBountyLead ? "true" : "false")
             << ", \"tradeIncidentBountyPublished\": " << (quest.tradeIncidentBountyPublished ? "true" : "false")
             << ", \"bountyMission\": " << (quest.bountyMission ? "true" : "false")
             << ", \"bountyTargetId\": \"" << escapeJson(quest.bountyTargetId) << "\""
             << ", \"bountyTargetName\": \"" << escapeJson(quest.bountyTargetName) << "\""
             << ", \"bountyTargetTemperament\": \"" << escapeJson(quest.bountyTargetTemperament) << "\""
             << ", \"bountyTargetHomeCityId\": \"" << escapeJson(quest.bountyTargetHomeCityId) << "\""
             << ", \"bountyTargetNetworkId\": \"" << escapeJson(quest.bountyTargetNetworkId) << "\""
             << ", \"bountyIssuerId\": \"" << escapeJson(quest.bountyIssuerId) << "\""
             << ", \"bountyIssuerName\": \"" << escapeJson(quest.bountyIssuerName) << "\""
             << ", \"bountyIssuerOrganizationId\": \"" << escapeJson(quest.bountyIssuerOrganizationId) << "\""
             << ", \"bountyIssuerOrganizationName\": \"" << escapeJson(quest.bountyIssuerOrganizationName) << "\""
             << ", \"bountyIssuerPrefersLiveCapture\": " << (quest.bountyIssuerPrefersLiveCapture ? "true" : "false")
             << ", \"bountyCloseRelationId\": \"" << escapeJson(quest.bountyCloseRelationId) << "\""
             << ", \"bountyCloseRelationName\": \"" << escapeJson(quest.bountyCloseRelationName) << "\""
             << ", \"bountyCloseRelationType\": \"" << escapeJson(quest.bountyCloseRelationType) << "\""
             << ", \"bountyStage\": \"" << escapeJson(quest.bountyStage) << "\""
             << ", \"bountyClues\": " << quest.bountyClues
             << ", \"bountyInvestigationNoise\": " << quest.bountyInvestigationNoise
             << ", \"bountyInformantExposure\": " << quest.bountyInformantExposure
             << ", \"bountyTargetAwareness\": " << quest.bountyTargetAwareness
             << ", \"bountyTargetSuspectsHunt\": " << (quest.bountyTargetSuspectsHunt ? "true" : "false")
             << ", \"bountyTargetKnowsHunt\": " << (quest.bountyTargetKnowsHunt ? "true" : "false")
             << ", \"bountyTargetResources\": " << quest.bountyTargetResources
             << ", \"bountyTargetLocalSupport\": " << quest.bountyTargetLocalSupport
             << ", \"bountyTargetMobility\": " << quest.bountyTargetMobility
             << ", \"bountyTargetEscapeAttempts\": " << quest.bountyTargetEscapeAttempts
             << ", \"bountyTargetKnowsTerrain\": " << (quest.bountyTargetKnowsTerrain ? "true" : "false")
             << ", \"bountyTargetInjured\": " << (quest.bountyTargetInjured ? "true" : "false")
             << ", \"bountyExitRecon\": " << quest.bountyExitRecon
             << ", \"bountyInformantNetwork\": " << quest.bountyInformantNetwork
             << ", \"bountyAmbushPreparation\": " << quest.bountyAmbushPreparation
             << ", \"bountyShadowingPreparation\": " << quest.bountyShadowingPreparation
             << ", \"bountyRouteVariation\": " << quest.bountyRouteVariation
             << ", \"bountyDecoyQuality\": " << quest.bountyDecoyQuality
             << ", \"bountyCounterSurveillance\": " << quest.bountyCounterSurveillance
             << ", \"bountyRefugeConcealment\": " << quest.bountyRefugeConcealment
             << ", \"bountyDangerRating\": " << quest.bountyDangerRating
             << ", \"bountyNotorietyRating\": " << quest.bountyNotorietyRating
             << ", \"bountyDaysAtLarge\": " << quest.bountyDaysAtLarge
             << ", \"bountyBaseRewardGold\": " << quest.bountyBaseRewardGold
             << ", \"bountyRewardProfileApplied\": " << (quest.bountyRewardProfileApplied ? "true" : "false")
             << ", \"bountyRegionalProfileApplied\": " << (quest.bountyRegionalProfileApplied ? "true" : "false")
             << ", \"bountyCurrentCityId\": \"" << escapeJson(quest.bountyCurrentCityId) << "\""
             << ", \"bountyCurrentBiome\": \"" << escapeJson(quest.bountyCurrentBiome) << "\""
             << ", \"bountyVisitedCityIds\": \"" << escapeJson(quest.bountyVisitedCityIds) << "\""
             << ", \"bountyRelocationCount\": " << quest.bountyRelocationCount
             << ", \"bountyTransitTrailKind\": \"" << escapeJson(quest.bountyTransitTrailKind) << "\""
             << ", \"bountyTransitTrailFromCityId\": \"" << escapeJson(quest.bountyTransitTrailFromCityId) << "\""
             << ", \"bountyTransitTrailReliability\": " << quest.bountyTransitTrailReliability
             << ", \"bountyTransitTrailAvailable\": " << (quest.bountyTransitTrailAvailable ? "true" : "false")
             << ", \"bountyRegionalProfileCityId\": \"" << escapeJson(quest.bountyRegionalProfileCityId) << "\""
             << ", \"bountyRegionalConsequencesApplied\": " << (quest.bountyRegionalConsequencesApplied ? "true" : "false")
             << ", \"bountyResolution\": \"" << escapeJson(quest.bountyResolution) << "\""
             << "}";
    };

    file << "  \"guildBoardState\": {\n";
    file << "    \"createdAtCombat\": " << player.getQuestLog().getGuildBoardCreatedAtCombat() << ",\n";
    file << "    \"targetSize\": " << player.getQuestLog().getGuildBoardTargetSize() << ",\n";
    file << "    \"pendingReplacements\": " << player.getQuestLog().getGuildBoardPendingReplacements() << ",\n";
    file << "    \"replacementDueAtCombat\": " << player.getQuestLog().getGuildBoardReplacementDueAtCombat() << "\n";
    file << "  },\n";

    file << "  \"guildBoardSnapshot\": [\n";
    const std::vector<Quest>& guildBoardOffers = player.getQuestLog().getGuildBoardOffers();
    for (std::size_t i = 0; i < guildBoardOffers.size(); i++)
    {
        writeQuestSnapshotEntry(guildBoardOffers[i], "    ");
        if (i + 1 < guildBoardOffers.size())
        {
            file << ",";
        }
        file << "\n";
    }
    file << "  ],\n";

    file << "  \"guildChallengeBoardState\": {\n";
    file << "    \"guildChallengeCreatedAtDay\": " << player.getQuestLog().getGuildChallengeBoardCreatedAtDay() << "\n";
    file << "  },\n";

    file << "  \"guildChallengeBoardSnapshot\": [\n";
    const std::vector<Quest>& guildChallengeBoardOffers = player.getQuestLog().getGuildChallengeBoardOffers();
    for (std::size_t i = 0; i < guildChallengeBoardOffers.size(); ++i)
    {
        writeQuestSnapshotEntry(guildChallengeBoardOffers[i], "    ");
        if (i + 1 < guildChallengeBoardOffers.size()) file << ",";
        file << "\n";
    }
    file << "  ],\n";

    file << "  \"questLogSnapshot\": [\n";

    const std::vector<Quest>& quests = player.getQuestLog().getQuests();

    for (std::size_t i = 0; i < quests.size(); i++)
    {
        writeQuestSnapshotEntry(quests[i], "    ");

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
    file << "    \"guildBoard\": \"persistent_offers_refresh_after_three_combats_replace_taken_after_one\",\n";
    file << "    \"pvpStatistics\": \"victories_defeats_lethal_eliminations_and_clone_status_saved_and_reloaded\",\n";
    file << "    \"skills\": \"passive_active_unlocks_and_weapon_progress_saved_and_reloaded\",\n";
    file << "    \"blessings\": \"prepared\"\n";
    file << "  }\n";
    file << "}\n";
    file.flush();
    const bool snapshotWriteOk = file.good();
    file.close();
    if (!snapshotWriteOk)
    {
        return false;
    }

    const long long currentCharacterPlaySeconds = std::max(0LL, player.getRealPlayTimeSeconds());
    const long long playDelta = std::max(0LL, currentCharacterPlaySeconds - std::max(0LL, previousCharacterPlaySeconds));
    if (playDelta > 0)
    {
        const std::string accountPath = getAccountSavePath(accountName);
        if (!std::filesystem::exists(accountPath))
        {
            saveAccountSnapshot(accountName);
        }
        std::string accountContent = readFileContent(accountPath);
        const long long oldAccountTotal = extractLongLongValue(accountContent, "totalRealPlayTimeSeconds", 0);
        const long long newAccountTotal = oldAccountTotal + playDelta;
        const std::string key = "\"totalRealPlayTimeSeconds\"";
        const std::size_t keyPos = accountContent.find(key);
        if (keyPos != std::string::npos)
        {
            const std::size_t colon = accountContent.find(':', keyPos);
            if (colon != std::string::npos)
            {
                std::size_t start = colon + 1;
                while (start < accountContent.size() && std::isspace(static_cast<unsigned char>(accountContent[start]))) ++start;
                std::size_t end = start;
                while (end < accountContent.size() && std::isdigit(static_cast<unsigned char>(accountContent[end]))) ++end;
                accountContent.replace(start, end - start, std::to_string(newAccountTotal));
                writeFileContent(accountPath, accountContent);
            }
        }
        else
        {
            const std::string statusLine = "  \"status\": \"active\",\n";
            const std::size_t statusPos = accountContent.find(statusLine);
            if (statusPos != std::string::npos)
            {
                accountContent.insert(statusPos + statusLine.size(),
                    "  \"totalRealPlayTimeSeconds\": " + std::to_string(newAccountTotal) + ",\n");
                writeFileContent(accountPath, accountContent);
            }
        }
    }

    // Legitimate characters advance the normal account trophy collection.
    // Altered characters may only synchronize the single explicit cheat trophy.
    if (!player.isAlteredByCheats())
    {
        if (!mergeAccountUnlockedTitles(accountName, player.getTitles(), player.getName()))
        {
            return false;
        }
    }
    else if (player.hasTitle("On ne dira rien..."))
    {
        if (!mergeAccountUnlockedTitles(accountName, {"On ne dira rien..."}, player.getName()))
        {
            return false;
        }
    }

    return true;
}

bool SaveManager::savePlayerExchangePair(
    const Player& firstPlayer,
    const std::string& firstAccountName,
    DifficultyMode firstDifficulty,
    DeathRuleMode firstDeathRule,
    const Player& secondPlayer,
    const std::string& secondAccountName,
    DifficultyMode secondDifficulty,
    DeathRuleMode secondDeathRule
)
{
    const std::string firstCharacterPath = getCharacterSavePath(firstAccountName, firstPlayer.getName());
    const std::string secondCharacterPath = getCharacterSavePath(secondAccountName, secondPlayer.getName());
    const std::string firstAccountPath = getAccountSavePath(firstAccountName);
    const std::string secondAccountPath = getAccountSavePath(secondAccountName);

    std::vector<SaveFileRollbackState> rollbackStates;
    auto captureUnique = [&](const std::string& path) {
        for (const SaveFileRollbackState& state : rollbackStates)
        {
            if (state.path == path)
            {
                return;
            }
        }
        rollbackStates.push_back(captureSaveFileState(path));
    };

    captureUnique(firstCharacterPath);
    captureUnique(secondCharacterPath);
    captureUnique(firstAccountPath);
    captureUnique(secondAccountPath);

    auto rollback = [&]() {
        bool restored = true;
        for (auto it = rollbackStates.rbegin(); it != rollbackStates.rend(); ++it)
        {
            restored = restoreSaveFileState(*it) && restored;
        }
        return restored;
    };

    try
    {
        if (!savePlayerSnapshot(firstPlayer, firstAccountName, firstDifficulty, firstDeathRule))
        {
            rollback();
            return false;
        }
        if (!savePlayerSnapshot(secondPlayer, secondAccountName, secondDifficulty, secondDeathRule))
        {
            rollback();
            return false;
        }
        return true;
    }
    catch (...)
    {
        rollback();
        return false;
    }
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
            summary.savedForVersion = extractStringValue(content, "gameVersion", "inconnue");
            summary.totalRealPlayTimeSeconds = extractLongLongValue(content, "totalRealPlayTimeSeconds", 0);
            summary.unlockedTitleCount = static_cast<int>(extractNamedStringObjectsFromArray(content, "accountUnlockedTitles").size());
            std::filesystem::file_time_type latestActivity = entry.last_write_time();
            summary.lastActivityText = formatLastActivityText(latestActivity);
            summary.playableCharacterCount = 0;

            const std::string safePrefix = buildSafeFileName(summary.accountName) + "__";
            const std::string characterFolder = SAVE_ROOT + "/characters/playable";
            try
            {
                for (const std::filesystem::directory_entry& characterEntry : std::filesystem::directory_iterator(characterFolder))
                {
                    if (!characterEntry.is_regular_file() || characterEntry.path().extension() != ".json") continue;
                    if (characterEntry.path().filename().string().rfind(safePrefix, 0) != 0) continue;
                    ++summary.playableCharacterCount;
                    const auto characterActivity = characterEntry.last_write_time();
                    if (characterActivity > latestActivity)
                    {
                        latestActivity = characterActivity;
                        summary.lastActivityText = formatLastActivityText(latestActivity);
                    }
                }
            }
            catch (...)
            {
                // The account still remains selectable if character metadata cannot be enumerated.
            }

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
    DeathRuleMode ignoredDeathRule = DeathRuleRules::defaultForDifficulty(summary.difficulty);
    return loadPlayerSnapshot(summary, player, difficulty, ignoredDeathRule);
}

bool SaveManager::loadPlayerSnapshot(
    const CharacterSaveSummary& summary,
    Player& player,
    DifficultyMode& difficulty,
    DeathRuleMode& deathRule
)
{
    std::string content = readFileContent(summary.path);

    if (content.empty())
    {
        return false;
    }

    const int loadedSaveVersion = extractIntValue(content, "saveVersion", 0);
    std::string characterName = extractStringValue(content, "name", summary.characterName);
    std::string className = extractStringValue(content, "class", summary.className);
    std::string raceName = extractStringValue(content, "race", summary.raceName);
    int characterAge = extractIntValue(content, "characterAge", summary.characterAge);
    std::string visualPresentation = extractStringValue(content, "visualPresentation", summary.visualPresentation);
    std::string visualVariant = extractStringValue(content, "visualVariant", summary.visualVariant);
    std::string bodyMorphology = extractStringValue(content, "bodyMorphology", "");
    std::string morphologyProgramTarget = extractStringValue(content, "morphologyProgramTarget", "");
    int morphologyProgramSessions = extractIntValue(content, "morphologyProgramSessions", 0);
    int morphologyProgramStartedDay = extractIntValue(content, "morphologyProgramStartedDay", -1);
    int morphologyProgramLastSessionDay = extractIntValue(content, "morphologyProgramLastSessionDay", -1);
    std::string createdAt = extractStringValue(content, "createdAt", "Inconnue");
    long long realPlayTimeSeconds = extractLongLongValue(content, "realPlayTimeSeconds", 0);
    std::string gameVersion = extractStringValue(content, "gameVersion", "inconnue");
    std::string createdForVersion = extractStringValue(content, "createdForVersion", gameVersion);
    std::string lastAdaptedVersion = extractStringValue(content, "lastAdaptedVersion", createdForVersion);
    std::string saveAccountName = extractStringValue(content, "account", summary.accountName);
    std::string creatorAccountName = extractStringValue(content, "creatorAccount", saveAccountName);
    std::string currentOwnerAccountName = extractStringValue(content, "currentOwnerAccount", saveAccountName);

    int level = extractIntValue(content, "level", 1);
    int experience = extractIntValue(content, "experience", 0);
    int hp = extractIntValue(content, "hp", 1);
    int loadedMaxHp = extractIntValue(content, "maxHp", 100);
    int loadedMana = extractIntValue(content, "mana", -1);
    int loadedManaExhaustionTurns = extractIntValue(content, "manaExhaustionTurns", 0);
    int loadedMinDamage = extractIntValue(content, "minDamage", 1);
    int loadedMaxDamage = extractIntValue(content, "maxDamage", 5);
    int loadedCriticalDamage = extractIntValue(content, "criticalDamage", 10);
    int gold = extractIntValue(content, "gold", 0);
    long long totalCopperCurrency = extractLongLongValue(content, "totalCopperCurrency", -1);
    const long long savedCopperCoins = extractLongLongValue(content, "currencyCopperCoins", -1);
    const long long savedIronCoins = extractLongLongValue(content, "currencyIronCoins", -1);
    const long long savedElectrumCoins = extractLongLongValue(content, "currencyElectrumCoins", -1);
    const long long savedGoldCoins = extractLongLongValue(content, "currencyGoldCoins", -1);
    const long long savedPlatinumCoins = extractLongLongValue(content, "currencyPlatinumCoins", -1);
    int unspentAttributePoints = extractIntValue(content, "unspentAttributePoints", 0);
    int strength = extractIntValue(content, "strength", 10);
    int dexterity = extractIntValue(content, "dexterity", 10);
    int constitution = extractIntValue(content, "constitution", 10);
    int intelligence = extractIntValue(content, "intelligence", 10);
    int wisdom = extractIntValue(content, "wisdom", 10);
    int charisma = extractIntValue(content, "charisma", 10);
    int equippedWeaponIndex = extractIntValue(content, "equippedWeaponIndex", -1);
    int equippedArmorIndex = extractIntValue(content, "equippedArmorIndex", -1);
    std::string interfaceHintFrequency = extractStringValue(content, "interfaceHintFrequency", "faible");
    bool revealHiddenTitles = extractBoolValue(content, "revealHiddenTitles", false);
    std::string ammunitionSelectionPolicy = extractStringValue(content, "ammunitionSelectionPolicy", "ask");
    std::string magicMaterialSelectionPolicy = extractStringValue(content, "magicMaterialSelectionPolicy", "ask");
    std::string lootPickupPolicy = extractStringValue(content, "lootPickupPolicy", "all");
    bool graphicalImagesEnabled = extractBoolValue(content, "graphicalImagesEnabled", false);
    bool storyModeStarted = extractBoolValue(content, "storyModeStarted", false);
    int storyChapter = extractIntValue(content, "storyChapter", 0);
    int storyStep = extractIntValue(content, "storyStep", 0);
    int storyCityDevelopmentLevel = extractIntValue(content, "storyCityDevelopmentLevel", 0);
    PlayerHomeState loadedHomeState;
    loadedHomeState.owned = extractBoolValue(content, "homeOwned", false);
    loadedHomeState.cityId = extractStringValue(content, "homeCityId", "");
    loadedHomeState.fullyPurchased = extractBoolValue(content, "homeFullyPurchased", false);
    loadedHomeState.rentAmount = std::max(0, extractIntValue(content, "homeRentAmount", 0));
    loadedHomeState.nextRentDueDay = extractIntValue(content, "homeNextRentDueDay", -1);
    loadedHomeState.npcSpouseName = extractStringValue(content, "homeNpcSpouseName", "");
    loadedHomeState.playerSpouseId = extractStringValue(content, "homePlayerSpouseId", "");
    loadedHomeState.playerMarriageSlotUsed = extractBoolValue(content, "homePlayerMarriageSlotUsed", false);
    PlayerHestiaStoryState loadedHestiaState;
    loadedHestiaState.rescued = extractBoolValue(content, "hestiaRescued", false);
    loadedHestiaState.materialized = extractBoolValue(content, "hestiaMaterialized", false);
    loadedHestiaState.residence = extractStringValue(content, "hestiaResidence", "unknown");
    loadedHestiaState.trust = extractIntValue(content, "hestiaTrust", 0);
    loadedHestiaState.awakeningStage = extractIntValue(content, "hestiaAwakeningStage", 0);
    loadedHestiaState.lastVisitDay = extractIntValue(content, "hestiaLastVisitDay", -1);
    loadedHestiaState.nextSmallFindDay = extractIntValue(content, "hestiaNextSmallFindDay", -1);
    loadedHestiaState.sacrificeConsumed = extractBoolValue(content, "hestiaSacrificeConsumed", false);
    PlayerHestiaArtifactState loadedHestiaArtifact;
    loadedHestiaArtifact.granted = extractBoolValue(content, "hestiaArtifactGranted", false);
    loadedHestiaArtifact.level = std::max(1, extractIntValue(content, "hestiaArtifactLevel", 1));
    loadedHestiaArtifact.killsTowardNextLevel = std::max(0, extractIntValue(content, "hestiaArtifactKillsTowardNextLevel", 0));
    loadedHestiaArtifact.primaryForm = extractStringValue(content, "hestiaArtifactPrimaryForm", "adaptive");
    loadedHestiaArtifact.secondaryForm = extractStringValue(content, "hestiaArtifactSecondaryForm", "");
    loadedHestiaArtifact.activeForm = extractStringValue(content, "hestiaArtifactActiveForm", "primary");
    loadedHestiaArtifact.secondFormUnlocked = extractBoolValue(content, "hestiaArtifactSecondFormUnlocked", loadedHestiaArtifact.level >= 16);
    for (const std::string& object : extractObjectsFromArray(content, "hestiaArtifactEssences"))
    {
        HestiaArtifactEssenceRecord essence;
        essence.familyId = extractStringValue(object, "familyId", "");
        essence.acceptedKills = std::max(0, extractIntValue(object, "acceptedKills", 0));
        if (!essence.familyId.empty()) loadedHestiaArtifact.absorbedEssences.push_back(essence);
    }
    int combatsStarted = extractIntValue(content, "combatsStarted", 0);
    int worldDaysElapsed = extractIntValue(content, "worldDaysElapsed", combatsStarted);
    int worldDayProgressUnits = extractIntValue(content, "worldDayProgressUnits", 0);
    int localSubscriptionRenewalPaidThisWeek = extractIntValue(content, "localSubscriptionRenewalPaidThisWeek", 0);
    int victories = extractIntValue(content, "victories", 0);
    int defeats = extractIntValue(content, "defeats", 0);
    int escapes = extractIntValue(content, "escapes", 0);
    int deaths = extractIntValue(content, "deaths", 0);
    int enemiesKilled = extractIntValue(content, "enemiesKilled", 0);
    int bossesKilled = extractIntValue(content, "bossesKilled", 0);
    int pvpVictories = extractIntValue(content, "pvpVictories", 0);
    int pvpDefeats = extractIntValue(content, "pvpDefeats", 0);

    difficulty = difficultyFromText(extractStringValue(content, "difficulty", "Normal"));
    deathRule = DeathRuleRules::fromSaveText(
        extractStringValue(content, "deathRule", DeathRuleRules::toSaveText(DeathRuleRules::defaultForDifficulty(difficulty))),
        difficulty
    );

    PlayerClass loadedClass = ClassCatalog::createClassByName(className);
    player = Player(characterName, loadedClass);
    player.setVersionMetadata(createdAt, createdForVersion, lastAdaptedVersion);
    player.setLoadedRealPlayTimeSeconds(realPlayTimeSeconds);
    player.setOwnershipMetadata(creatorAccountName, currentOwnerAccountName);
    player.setRace(raceFromText(raceName));
    player.setAppearanceProfile(characterAge, visualPresentation, visualVariant);
    player.setLoadedMorphologyProgramState(bodyMorphology, morphologyProgramTarget, morphologyProgramSessions, morphologyProgramStartedDay, morphologyProgramLastSessionDay);
    player.initializeStarterInventory(difficulty);
    player.setClone(extractBoolValue(content, "clone", false));
    player.setInterfaceHintFrequency(interfaceHintFrequency);
    player.setRevealHiddenTitles(revealHiddenTitles);
    player.setAmmunitionSelectionPolicy(ammunitionSelectionPolicy);
    player.setMagicMaterialSelectionPolicy(magicMaterialSelectionPolicy);
    player.setGraphicalImagesEnabled(graphicalImagesEnabled);
    player.setLoadedStoryProgress(storyChapter, storyStep, storyCityDevelopmentLevel, storyModeStarted);
    player.setLoadedHomeState(loadedHomeState);
    player.setLoadedHestiaStoryState(loadedHestiaState);
    player.setLoadedHestiaArtifactState(loadedHestiaArtifact);

    std::vector<std::string> weaponObjects = extractObjectsFromArray(content, "weapons");
    std::vector<std::string> armorObjects = extractObjectsFromArray(content, "armors");
    std::vector<std::string> consumableObjects = extractObjectsFromArray(content, "consumables");
    std::vector<std::string> materialObjects = extractObjectsFromArray(content, "materials");
    std::vector<std::string> pinnedItemKeyObjects = extractObjectsFromArray(content, "pinnedItemKeys");
    std::vector<std::string> favoriteAmmunitionIdObjects = extractObjectsFromArray(content, "favoriteAmmunitionIds");
    std::vector<std::string> legacyFavoriteItemKeyObjects = extractObjectsFromArray(content, "favoriteItemKeys");
    std::vector<std::string> lockedItemKeyObjects = extractObjectsFromArray(content, "lockedItemKeys");
    std::vector<std::string> trackedMaterialIdObjects = extractObjectsFromArray(content, "trackedMaterialIds");
    std::vector<std::string> ignoredLootCategoryObjects = extractObjectsFromArray(content, "ignoredLootCategoryIds");
    std::vector<std::string> ignoredLootMaterialObjects = extractObjectsFromArray(content, "ignoredLootMaterialIds");
    std::vector<std::string> materialKnowledgeObjects = extractObjectsFromArray(content, "materialKnowledgeSnapshot");
    std::vector<std::string> vaultWeaponObjects = extractObjectsFromArray(content, "vaultWeapons");
    std::vector<std::string> vaultArmorObjects = extractObjectsFromArray(content, "vaultArmors");
    std::vector<std::string> vaultConsumableObjects = extractObjectsFromArray(content, "vaultConsumables");
    std::vector<std::string> vaultMaterialObjects = extractObjectsFromArray(content, "vaultMaterials");
    std::vector<std::string> cityVaultObjects = extractObjectsFromArray(content, "cityVaults");
    std::vector<std::string> cityVaultWeaponObjects = extractObjectsFromArray(content, "cityVaultWeapons");
    std::vector<std::string> cityVaultArmorObjects = extractObjectsFromArray(content, "cityVaultArmors");
    std::vector<std::string> cityVaultConsumableObjects = extractObjectsFromArray(content, "cityVaultConsumables");
    std::vector<std::string> cityVaultMaterialObjects = extractObjectsFromArray(content, "cityVaultMaterials");
    std::vector<std::string> battlefieldRecoveryCacheObjects = extractObjectsFromArray(content, "battlefieldRecoveryCaches");
    std::vector<std::string> battlefieldRecoveryWeaponObjects = extractObjectsFromArray(content, "battlefieldRecoveryWeapons");
    std::vector<std::string> battlefieldRecoveryArmorObjects = extractObjectsFromArray(content, "battlefieldRecoveryArmors");
    std::vector<std::string> battlefieldRecoveryConsumableObjects = extractObjectsFromArray(content, "battlefieldRecoveryConsumables");
    std::vector<std::string> battlefieldRecoveryMaterialObjects = extractObjectsFromArray(content, "battlefieldRecoveryMaterials");
    std::vector<std::string> crimeContactStateObjects = extractObjectsFromArray(content, "crimeContactStates");
    std::vector<std::string> vaultTheftCaseObjects = extractObjectsFromArray(content, "vaultTheftCases");
    std::vector<std::string> vaultTheftCaseWeaponObjects = extractObjectsFromArray(content, "vaultTheftCaseWeapons");
    std::vector<std::string> vaultTheftCaseArmorObjects = extractObjectsFromArray(content, "vaultTheftCaseArmors");
    std::vector<std::string> registeredGuildCityObjects = extractObjectsFromArray(content, "registeredGuildCityIds");
    std::vector<std::string> bestiaryObjects = extractObjectsFromArray(content, "bestiaryRuntimeSnapshot");
    std::vector<std::string> questObjects = extractObjectsFromArray(content, "questLogSnapshot");
    std::vector<std::string> guildBoardObjects = extractObjectsFromArray(content, "guildBoardSnapshot");
    std::vector<std::string> guildChallengeBoardObjects = extractObjectsFromArray(content, "guildChallengeBoardSnapshot");
    std::vector<std::string> lethalEliminationObjects = extractObjectsFromArray(content, "pvpLethalEliminations");
    std::vector<std::string> localSubscriptionObjects = extractObjectsFromArray(content, "localSubscriptions");
    std::vector<std::string> activeBlessingObjects = extractObjectsFromArray(content, "activeBlessings");
    std::vector<std::string> activeCurseObjects = extractObjectsFromArray(content, "activeCurses");
    std::vector<std::string> starterKitObjects = extractObjectsFromArray(content, "starterKitSnapshot");
    std::vector<std::string> titleObjects = extractObjectsFromArray(content, "titles");
    std::vector<std::string> activeTitleObjects = extractObjectsFromArray(content, "activeTitles");
    std::vector<std::string> recentEquipmentUsageObjects = extractObjectsFromArray(content, "recentCombatEquipmentUsage");
    std::vector<std::string> passiveSkillObjects = extractObjectsFromArray(content, "unlockedPassiveSkills");
    std::vector<std::string> activeSkillObjects = extractObjectsFromArray(content, "unlockedActiveSkills");
    std::vector<std::string> enabledPassiveSkillObjects = extractObjectsFromArray(content, "enabledPassiveSkills");
    std::vector<std::string> equippedActiveSkillObjects = extractObjectsFromArray(content, "equippedActiveSkills");
    std::vector<std::string> unlockedBossObjects = extractObjectsFromArray(content, "unlockedBossIds");
    int recentBossCooldownExpiresAtDay = extractIntValue(content, "recentBossCooldownExpiresAtDay", -1);
    int rareBossDiscoveryCooldownExpiresAtDay = extractIntValue(content, "rareBossDiscoveryCooldownExpiresAtDay", -1);
    std::vector<std::string> recentBossObjects = extractObjectsFromArray(content, "recentBossIds");
    std::vector<std::string> defeatedBossObjects = extractObjectsFromArray(content, "defeatedBossIds");
    std::vector<std::string> bossDiscoveryLocationObjects = extractObjectsFromArray(content, "bossDiscoveryLocations");
    std::vector<std::string> recentExplorationEventObjects = extractObjectsFromArray(content, "recentEventKeys");
    std::vector<std::string> recentExplorationChallengeObjects = extractObjectsFromArray(content, "recentChallengeKeys");
    std::vector<std::string> explorationSceneCooldownObjects = extractObjectsFromArray(content, "sceneCooldowns");
    std::vector<std::string> shopPromotionPurchaseObjects = extractObjectsFromArray(content, "promotionPurchases");
    std::vector<std::string> canonicalJournalObjects = extractObjectsFromArray(content, "canonicalJournalSnapshot");
    std::vector<std::string> historicalEventObjects = extractObjectsFromArray(content, "historicalEvents");
    std::vector<std::string> rivalObjects = extractObjectsFromArray(content, "rivals");
    std::vector<std::string> npcKnownFactObjects = extractObjectsFromArray(content, "npcKnownFacts");
    std::vector<std::string> languageKnowledgeObjects = extractObjectsFromArray(content, "languageKnowledge");
    std::vector<std::string> grinkaStolenWeaponObjects = extractObjectsFromArray(content, "grinkaStolenWeapons");
    std::vector<std::string> grinkaStolenArmorObjects = extractObjectsFromArray(content, "grinkaStolenArmors");

    std::vector<NpcKnownFact> loadedNpcKnownFacts;
    for (const std::string& object : npcKnownFactObjects)
    {
        NpcKnownFact fact;
        fact.npcId = extractStringValue(object, "npcId", "");
        fact.factType = extractStringValue(object, "factType", "");
        fact.subjectId = extractStringValue(object, "subjectId", "");
        fact.label = extractStringValue(object, "label", "");
        fact.sourceType = extractStringValue(object, "sourceType", "");
        fact.sourceId = extractStringValue(object, "sourceId", "");
        fact.locationId = extractStringValue(object, "locationId", "");
        fact.claimVariant = extractStringValue(object, "claimVariant", "default");
        fact.relayChannel = extractStringValue(object, "relayChannel", "");
        fact.firstLearnedDay = extractIntValue(object, "firstLearnedDay", 0);
        fact.lastReinforcedDay = extractIntValue(object, "lastReinforcedDay", fact.firstLearnedDay);
        fact.confidence = extractIntValue(object, "confidence", 0);
        fact.evidenceLevel = extractIntValue(object, "evidenceLevel", 0);
        fact.timesHeard = extractIntValue(object, "timesHeard", 1);
        if (!fact.npcId.empty() && !fact.factType.empty()) loadedNpcKnownFacts.push_back(fact);
    }
    player.setLoadedNpcKnownFacts(loadedNpcKnownFacts);

    std::vector<PlayerLanguageKnowledge> loadedLanguageKnowledge;
    for (const std::string& object : languageKnowledgeObjects)
    {
        PlayerLanguageKnowledge language;
        language.languageId = extractStringValue(object, "id", "");
        language.level = extractIntValue(object, "level", 0);
        language.studyProgress = extractIntValue(object, "studyProgress", 0);
        language.nativeLanguage = extractBoolValue(object, "native", false);
        if (!language.languageId.empty()) loadedLanguageKnowledge.push_back(language);
    }
    player.setLoadedLanguageKnowledge(loadedLanguageKnowledge);

    MaterialKnowledgeProgress::clear();
    for (const std::string& object : materialKnowledgeObjects)
    {
        MaterialKnowledgeRecord record;
        record.id = extractStringValue(object, "id", "");
        record.name = extractStringValue(object, "name", "");
        record.category = extractStringValue(object, "category", "");
        record.quality = extractStringValue(object, "quality", "normal");
        record.discoveredQuantity = extractIntValue(object, "discoveredQuantity", 0);
        record.bestQualityWeight = extractIntValue(object, "bestQualityWeight", 0);
        MaterialKnowledgeProgress::restoreRecord(record);
    }

    if (!weaponObjects.empty() || !armorObjects.empty() || !consumableObjects.empty() || !materialObjects.empty())
    {
        player.unequipWeapon();
        player.unequipArmor();
        // Loading reconstructs existing possessions; it must never look like fresh tracked loot.
        // Clear any in-memory preferences first, then restore the saved preference block below.
        player.getInventory().setLoadedInventoryPreferences({}, {}, {}, {});
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
            weapon.loadEnchantmentsFromSaveText(extractStringValue(object, "enchantments", ""));
        weapon.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(weapon, object);
            weapon.setPersistentId(extractStringValue(object, "persistentId", ""));
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
            armor.loadEnchantmentsFromSaveText(extractStringValue(object, "enchantments", ""));
        armor.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(armor, object);
            armor.setPersistentId(extractStringValue(object, "persistentId", ""));
            player.getInventory().addArmor(armor);
        }

        for (const std::string& object : consumableObjects)
        {
            Consumable consumable = consumableFromName(extractStringValue(object, "name", "Potion de soin"));
            consumable.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(consumable, object);
            player.getInventory().addConsumable(consumable);
        }

        for (const std::string& object : materialObjects)
        {
            std::string id = extractStringValue(object, "id", "unknown_material");
            int quantity = extractIntValue(object, "quantity", 1);
            std::string quality = extractStringValue(object, "quality", "normal");
            Material material = MaterialCatalog::createById(id, quantity, quality);
            material.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(material, object);
            player.getInventory().addMaterial(material);
        }
    }

    std::vector<std::string> loadedPinnedItemKeys;
    for (const std::string& object : pinnedItemKeyObjects)
    {
        const std::string key = extractStringValue(object, "key", "");
        if (!key.empty()) loadedPinnedItemKeys.push_back(key);
    }
    std::vector<std::string> loadedFavoriteAmmunitionIds;
    for (const std::string& object : favoriteAmmunitionIdObjects)
    {
        const std::string id = extractStringValue(object, "id", "");
        if (Inventory::isAmmunitionMaterialId(id)) loadedFavoriteAmmunitionIds.push_back(id);
    }

    // V3.51.16 compatibility: generic favorites meant both sorting and automatic choices.
    // Preserve their visible intent as pins, and only migrate real ammunition to ammunition favorites.
    if (content.find("\"pinnedItemKeys\"") == std::string::npos)
    {
        for (const std::string& object : legacyFavoriteItemKeyObjects)
        {
            const std::string key = extractStringValue(object, "key", "");
            if (key.empty()) continue;
            loadedPinnedItemKeys.push_back(key);
            const std::string materialPrefix = "material:";
            if (key.rfind(materialPrefix, 0) == 0)
            {
                const std::string id = key.substr(materialPrefix.size());
                if (Inventory::isAmmunitionMaterialId(id)) loadedFavoriteAmmunitionIds.push_back(id);
            }
        }
    }
    std::vector<std::string> loadedLockedItemKeys;
    for (const std::string& object : lockedItemKeyObjects)
    {
        const std::string key = extractStringValue(object, "key", "");
        if (!key.empty()) loadedLockedItemKeys.push_back(key);
    }
    std::vector<std::string> loadedTrackedMaterialIds;
    for (const std::string& object : trackedMaterialIdObjects)
    {
        const std::string id = extractStringValue(object, "id", "");
        if (!id.empty()) loadedTrackedMaterialIds.push_back(id);
    }
    player.getInventory().setLoadedInventoryPreferences(loadedPinnedItemKeys, loadedLockedItemKeys, loadedTrackedMaterialIds, loadedFavoriteAmmunitionIds);

    std::vector<std::string> loadedIgnoredLootCategoryIds;
    for (const std::string& object : ignoredLootCategoryObjects)
    {
        const std::string id = extractStringValue(object, "id", "");
        if (!id.empty()) loadedIgnoredLootCategoryIds.push_back(id);
    }
    std::vector<std::string> loadedIgnoredLootMaterialIds;
    for (const std::string& object : ignoredLootMaterialObjects)
    {
        const std::string id = extractStringValue(object, "id", "");
        if (!id.empty()) loadedIgnoredLootMaterialIds.push_back(id);
    }
    player.setLoadedLootPreferences(lootPickupPolicy, loadedIgnoredLootCategoryIds, loadedIgnoredLootMaterialIds);

    std::vector<std::string> loadedStarterKitLog;
    for (const std::string& object : starterKitObjects)
    {
        loadedStarterKitLog.push_back(extractStringValue(object, "label", "Entrée de départ inconnue"));
    }
    player.setLoadedStarterKitLog(loadedStarterKitLog);

    std::vector<std::string> loadedTitles;
    for (const std::string& object : titleObjects)
    {
        std::string titleName = extractStringValue(object, "name", "");
        if (!titleName.empty())
        {
            loadedTitles.push_back(titleName);
        }
    }

    std::vector<std::string> loadedActiveTitles;
    for (const std::string& object : activeTitleObjects)
    {
        std::string titleName = extractStringValue(object, "name", "");
        if (!titleName.empty())
        {
            loadedActiveTitles.push_back(titleName);
        }
    }
    player.setLoadedTitles(loadedTitles, extractStringValue(content, "activeTitle", ""), loadedActiveTitles);

    Inventory loadedCityVault;
    for (const std::string& object : vaultWeaponObjects)
    {
        const std::string weaponName = extractStringValue(object, "name", "Épée rouillée");
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
        weapon.loadEnchantmentsFromSaveText(extractStringValue(object, "enchantments", ""));
        weapon.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(weapon, object);
        weapon.setPersistentId(extractStringValue(object, "persistentId", ""));
        loadedCityVault.addWeapon(weapon);
    }
    for (const std::string& object : vaultArmorObjects)
    {
        const std::string armorName = extractStringValue(object, "name", "Tenue simple");
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
        armor.loadEnchantmentsFromSaveText(extractStringValue(object, "enchantments", ""));
        armor.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(armor, object);
        armor.setPersistentId(extractStringValue(object, "persistentId", ""));
        loadedCityVault.addArmor(armor);
    }
    for (const std::string& object : vaultConsumableObjects)
    {
        Consumable consumable = consumableFromName(extractStringValue(object, "name", "Potion de soin"));
        consumable.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(consumable, object);
        loadedCityVault.addConsumable(consumable);
    }
    for (const std::string& object : vaultMaterialObjects)
    {
        Material material = MaterialCatalog::createById(
            extractStringValue(object, "id", "unknown_material"),
            extractIntValue(object, "quantity", 1),
            extractStringValue(object, "quality", "normal")
        );
        material.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(material, object);
        loadedCityVault.addMaterial(material);
    }
    std::vector<PlayerCityVault> loadedCityVaults;
    auto findOrCreateLoadedCityVault = [&](const std::string& rawCityId) -> PlayerCityVault&
    {
        const std::string cityId = rawCityId.empty() ? extractStringValue(content, "currentCityId", "valebrume") : rawCityId;
        for (PlayerCityVault& record : loadedCityVaults)
        {
            if (record.cityId == cityId)
            {
                return record;
            }
        }

        PlayerCityVault record;
        record.cityId = cityId;
        record.purchased = false;
        record.level = 0;
        loadedCityVaults.push_back(record);
        return loadedCityVaults.back();
    };

    for (const std::string& object : cityVaultObjects)
    {
        const std::string cityId = extractStringValue(object, "cityId", "");
        if (cityId.empty())
        {
            continue;
        }
        PlayerCityVault& record = findOrCreateLoadedCityVault(cityId);
        record.purchased = extractBoolValue(object, "purchased", false);
        record.level = record.purchased ? std::max(1, std::min(5, extractIntValue(object, "level", 1))) : 0;
        record.placeOptimized = record.purchased && extractBoolValue(object, "placeOptimized", false);
        record.preparedStorage = record.purchased && record.level >= 5 && extractBoolValue(object, "preparedStorage", false);
        record.securityDaysRemaining = record.purchased ? std::max(0, extractIntValue(object, "securityDaysRemaining", 0)) : 0;
        record.raidPending = record.purchased && extractBoolValue(object, "raidPending", false);
        record.raidDeadlineDay = record.raidPending ? extractIntValue(object, "raidDeadlineDay", -1) : -1;
        record.lastRaidCheckDay = extractIntValue(object, "lastRaidCheckDay", -1);
        CoinBreakdown storedCoins;
        storedCoins.platinum = std::max<long long>(0, extractLongLongValue(object, "platinumCoins", 0));
        storedCoins.gold = std::max<long long>(0, extractLongLongValue(object, "goldCoins", 0));
        storedCoins.electrum = std::max<long long>(0, extractLongLongValue(object, "electrumCoins", 0));
        storedCoins.iron = std::max<long long>(0, extractLongLongValue(object, "ironCoins", 0));
        storedCoins.copper = std::max<long long>(0, extractLongLongValue(object, "copperCoins", 0));
        record.inventory.setCoinStacks(storedCoins);
    }

    for (const std::string& object : cityVaultWeaponObjects)
    {
        const std::string cityId = extractStringValue(object, "cityId", "");
        if (cityId.empty()) continue;
        PlayerCityVault& record = findOrCreateLoadedCityVault(cityId);
        record.purchased = true;
        if (record.level <= 0) record.level = 1;
        const std::string weaponName = extractStringValue(object, "name", "Épée rouillée");
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
        weapon.loadEnchantmentsFromSaveText(extractStringValue(object, "enchantments", ""));
        weapon.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(weapon, object);
        weapon.setPersistentId(extractStringValue(object, "persistentId", ""));
        record.inventory.addWeapon(weapon);
    }

    for (const std::string& object : cityVaultArmorObjects)
    {
        const std::string cityId = extractStringValue(object, "cityId", "");
        if (cityId.empty()) continue;
        PlayerCityVault& record = findOrCreateLoadedCityVault(cityId);
        record.purchased = true;
        if (record.level <= 0) record.level = 1;
        const std::string armorName = extractStringValue(object, "name", "Tenue simple");
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
        armor.loadEnchantmentsFromSaveText(extractStringValue(object, "enchantments", ""));
        armor.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(armor, object);
        armor.setPersistentId(extractStringValue(object, "persistentId", ""));
        record.inventory.addArmor(armor);
    }

    for (const std::string& object : cityVaultConsumableObjects)
    {
        const std::string cityId = extractStringValue(object, "cityId", "");
        if (cityId.empty()) continue;
        PlayerCityVault& record = findOrCreateLoadedCityVault(cityId);
        record.purchased = true;
        if (record.level <= 0) record.level = 1;
        Consumable consumable = consumableFromName(extractStringValue(object, "name", "Potion de soin"));
        consumable.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(consumable, object);
        record.inventory.addConsumable(consumable);
    }

    for (const std::string& object : cityVaultMaterialObjects)
    {
        const std::string cityId = extractStringValue(object, "cityId", "");
        if (cityId.empty()) continue;
        PlayerCityVault& record = findOrCreateLoadedCityVault(cityId);
        record.purchased = true;
        if (record.level <= 0) record.level = 1;
        Material material = MaterialCatalog::createById(
            extractStringValue(object, "id", "unknown_material"),
            extractIntValue(object, "quantity", 1),
            extractStringValue(object, "quality", "normal")
        );
        material.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(material, object);
        record.inventory.addMaterial(material);
    }

    std::vector<PlayerBattlefieldRecoveryCache> loadedBattlefieldCaches;
    auto findOrCreateBattlefieldCache = [&](const std::string& cacheId) -> PlayerBattlefieldRecoveryCache&
    {
        for (PlayerBattlefieldRecoveryCache& cache : loadedBattlefieldCaches) if (cache.cacheId == cacheId) return cache;
        PlayerBattlefieldRecoveryCache cache; cache.cacheId = cacheId; loadedBattlefieldCaches.push_back(cache); return loadedBattlefieldCaches.back();
    };
    for (const std::string& object : battlefieldRecoveryCacheObjects)
    {
        const std::string cacheId = extractStringValue(object, "cacheId", "");
        if (cacheId.empty()) continue;
        PlayerBattlefieldRecoveryCache& cache = findOrCreateBattlefieldCache(cacheId);
        cache.cityId = extractStringValue(object, "cityId", "");
        cache.contextId = extractStringValue(object, "contextId", "");
        cache.locationLabel = extractStringValue(object, "locationLabel", "zone du combat");
        cache.createdDay = std::max(0, extractIntValue(object, "createdDay", 0));
        cache.lastAttemptDay = extractIntValue(object, "lastAttemptDay", -1);
        cache.searchAttempts = std::max(0, extractIntValue(object, "searchAttempts", 0));
        cache.disturbanceLevel = std::clamp(extractIntValue(object, "disturbanceLevel", 0), 0, 3);
        cache.custodyState = extractStringValue(object, "custodyState", "field");
        cache.custodyLabel = extractStringValue(object, "custodyLabel", "");
        cache.lastMovementDay = extractIntValue(object, "lastMovementDay", -1);
        cache.movementCount = std::max(0, extractIntValue(object, "movementCount", 0));
        cache.locationKnown = extractBoolValue(object, "locationKnown", true);
        cache.resolved = extractBoolValue(object, "resolved", false);
        CoinBreakdown coins;
        coins.platinum = std::max<long long>(0, extractLongLongValue(object, "platinumCoins", 0));
        coins.gold = std::max<long long>(0, extractLongLongValue(object, "goldCoins", 0));
        coins.electrum = std::max<long long>(0, extractLongLongValue(object, "electrumCoins", 0));
        coins.iron = std::max<long long>(0, extractLongLongValue(object, "ironCoins", 0));
        coins.copper = std::max<long long>(0, extractLongLongValue(object, "copperCoins", 0));
        cache.inventory.setCoinStacks(coins);
    }
    for (const std::string& object : battlefieldRecoveryWeaponObjects)
    {
        const std::string cacheId = extractStringValue(object, "cacheId", "");
        if (cacheId.empty()) continue;
        PlayerBattlefieldRecoveryCache& cache = findOrCreateBattlefieldCache(cacheId);
        const std::string name = extractStringValue(object, "name", "Épée rouillée");
        Weapon fallback = weaponFromName(name);
        Weapon w(name, extractStringValue(object, "description", fallback.getDescription()), extractIntValue(object, "value", fallback.getValue()),
            weaponTypeFromSaveText(extractStringValue(object, "type", weaponTypeToSaveText(fallback.getType()))),
            extractIntValue(object, "minDamageBonus", fallback.getMinDamageBonus()), extractIntValue(object, "maxDamageBonus", fallback.getMaxDamageBonus()),
            extractIntValue(object, "criticalBonus", fallback.getCriticalBonus()), extractIntValue(object, "maxDurability", fallback.getMaxDurability()));
        applySavedWeaponDurability(w, extractIntValue(object, "durability", w.getDurability()));
        w.loadEnchantmentsFromSaveText(extractStringValue(object, "enchantments", ""));
        w.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(w, object);
        w.setPersistentId(extractStringValue(object, "persistentId", ""));
        cache.inventory.addWeapon(w);
    }
    for (const std::string& object : battlefieldRecoveryArmorObjects)
    {
        const std::string cacheId = extractStringValue(object, "cacheId", "");
        if (cacheId.empty()) continue;
        PlayerBattlefieldRecoveryCache& cache = findOrCreateBattlefieldCache(cacheId);
        const std::string name = extractStringValue(object, "name", "Tenue simple");
        Armor fallback = armorFromName(name);
        Armor a(name, extractStringValue(object, "description", fallback.getDescription()), extractIntValue(object, "value", fallback.getValue()),
            armorTypeFromSaveText(extractStringValue(object, "type", armorTypeToSaveText(fallback.getType()))),
            extractIntValue(object, "maxHpBonus", fallback.getMaxHpBonus()), extractIntValue(object, "damageReduction", fallback.getDamageReduction()),
            extractIntValue(object, "maxDurability", fallback.getMaxDurability()));
        applySavedArmorDurability(a, extractIntValue(object, "durability", a.getDurability()));
        a.loadEnchantmentsFromSaveText(extractStringValue(object, "enchantments", ""));
        a.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(a, object);
        a.setPersistentId(extractStringValue(object, "persistentId", ""));
        cache.inventory.addArmor(a);
    }
    for (const std::string& object : battlefieldRecoveryConsumableObjects)
    {
        const std::string cacheId = extractStringValue(object, "cacheId", "");
        if (cacheId.empty()) continue;
        PlayerBattlefieldRecoveryCache& cache = findOrCreateBattlefieldCache(cacheId);
        Consumable c = consumableFromName(extractStringValue(object, "name", "Potion de soin"));
        c.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(c, object);
        cache.inventory.addConsumable(c);
    }
    for (const std::string& object : battlefieldRecoveryMaterialObjects)
    {
        const std::string cacheId = extractStringValue(object, "cacheId", "");
        if (cacheId.empty()) continue;
        PlayerBattlefieldRecoveryCache& cache = findOrCreateBattlefieldCache(cacheId);
        Material m = MaterialCatalog::createById(extractStringValue(object, "id", "unknown_material"),
            std::max(1, extractIntValue(object, "quantity", 1)), extractStringValue(object, "quality", "normal"));
        m.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(m, object);
        cache.inventory.addMaterial(m);
    }

    std::vector<PlayerCrimeContactState> loadedCrimeContacts;
    for (const std::string& object : crimeContactStateObjects)
    {
        PlayerCrimeContactState state;
        state.contactId = extractStringValue(object, "contactId", "");
        if (state.contactId.empty()) continue;
        state.displayName = extractStringValue(object, "displayName", state.contactId);
        state.firstSeenDay = std::max(0, extractIntValue(object, "firstSeenDay", 0));
        state.lastInteractionDay = extractIntValue(object, "lastInteractionDay", -1);
        state.encounters = std::max(0, extractIntValue(object, "encounters", 0));
        state.observations = std::max(0, extractIntValue(object, "observations", 0));
        state.negotiations = std::max(0, extractIntValue(object, "negotiations", 0));
        state.guardConfrontations = std::max(0, extractIntValue(object, "guardConfrontations", 0));
        state.pressure = std::clamp(extractIntValue(object, "pressure", 0), 0, 100);
        state.cooperation = std::clamp(extractIntValue(object, "cooperation", 0), -100, 100);
        state.fledUntilDay = extractIntValue(object, "fledUntilDay", -1);
        state.detainedUntilDay = extractIntValue(object, "detainedUntilDay", -1);
        state.provisionalDetentions = std::max(0, extractIntValue(object, "provisionalDetentions", 0));
        state.releases = std::max(0, extractIntValue(object, "releases", 0));
        state.returnsAfterFlight = std::max(0, extractIntValue(object, "returnsAfterFlight", 0));
        state.lastReleaseDay = extractIntValue(object, "lastReleaseDay", -1);
        state.lastReturnDay = extractIntValue(object, "lastReturnDay", -1);
        state.knownRole = extractStringValue(object, "knownRole", "");
        state.homeCityId = extractStringValue(object, "homeCityId", "");
        state.networkId = extractStringValue(object, "networkId", "");
        state.networkWarnings = std::max(0, extractIntValue(object, "networkWarnings", 0));
        state.lastNetworkWarningDay = extractIntValue(object, "lastNetworkWarningDay", -1);
        state.lastWarningSourceId = extractStringValue(object, "lastWarningSourceId", "");
        state.reliability = std::clamp(extractIntValue(object, "reliability", 50), 0, 100);
        state.favorBalance = std::clamp(extractIntValue(object, "favorBalance", 0), -100, 100);
        state.rivalry = std::clamp(extractIntValue(object, "rivalry", 0), 0, 100);
        state.fulfilledDeals = std::max(0, extractIntValue(object, "fulfilledDeals", 0));
        state.brokenDeals = std::max(0, extractIntValue(object, "brokenDeals", 0));
        state.ruptureUntilDay = extractIntValue(object, "ruptureUntilDay", -1);
        state.reportedToGuard = extractBoolValue(object, "reportedToGuard", false);
        state.arrested = extractBoolValue(object, "arrested", false);
        loadedCrimeContacts.push_back(state);
    }

    std::vector<PlayerVaultTheftCase> loadedVaultTheftCases;
    auto findOrCreateTheftCase = [&](const std::string& caseId) -> PlayerVaultTheftCase&
    {
        for (PlayerVaultTheftCase& c : loadedVaultTheftCases) if (c.caseId == caseId) return c;
        PlayerVaultTheftCase c; c.caseId = caseId; loadedVaultTheftCases.push_back(c); return loadedVaultTheftCases.back();
    };
    for (const std::string& object : vaultTheftCaseObjects)
    {
        const std::string caseId = extractStringValue(object, "caseId", "");
        if (caseId.empty()) continue;
        PlayerVaultTheftCase& c = findOrCreateTheftCase(caseId);
        c.cityId = extractStringValue(object, "cityId", "");
        c.createdDay = extractIntValue(object, "createdDay", 0);
        c.lastLeadDay = extractIntValue(object, "lastLeadDay", c.createdDay);
        c.clues = extractIntValue(object, "clues", 0);
        c.culpritIdentified = extractBoolValue(object, "culpritIdentified", false);
        c.identifiedRivalId = extractStringValue(object, "identifiedRivalId", "");
        c.holderType = extractStringValue(object, "holderType", "unknown");
        c.holderLabel = extractStringValue(object, "holderLabel", "auteur inconnu");
        c.holderWorldContactId = extractStringValue(object, "holderWorldContactId", "");
        c.holderLocationId = extractStringValue(object, "holderLocationId", "");
        c.holderLocationLabel = extractStringValue(object, "holderLocationLabel", "");
        c.holderExplorationBiome = extractStringValue(object, "holderExplorationBiome", "");
        c.handoffCount = extractIntValue(object, "handoffCount", 0);
        c.holderObservationCount = extractIntValue(object, "holderObservationCount", 0);
        c.negotiationAttempts = extractIntValue(object, "negotiationAttempts", 0);
        c.confrontationAttempts = extractIntValue(object, "confrontationAttempts", 0);
        c.holderPressure = extractIntValue(object, "holderPressure", 0);
        c.holderCooperation = extractIntValue(object, "holderCooperation", 0);
        c.holderClosedUntilDay = extractIntValue(object, "holderClosedUntilDay", -1);
        c.holderReportedToGuard = extractBoolValue(object, "holderReportedToGuard", false);
        c.lastHolderInteractionDay = extractIntValue(object, "lastHolderInteractionDay", -1);
        c.holderAlerted = extractBoolValue(object, "holderAlerted", false);
        c.resolved = extractBoolValue(object, "resolved", false);
        c.stolenCopper = std::max<long long>(0, extractLongLongValue(object, "stolenCopper", 0));
        c.hasWeapon = extractBoolValue(object, "hasWeapon", false);
        c.hasArmor = extractBoolValue(object, "hasArmor", false);
        c.hasConsumable = extractBoolValue(object, "hasConsumable", false);
        if (c.hasConsumable)
        {
            c.stolenConsumable = consumableFromName(extractStringValue(object, "stolenConsumableName", "Potion de soin"));
            c.stolenConsumable.loadOwnershipBindingFromSaveText(extractStringValue(object, "stolenConsumableOwnershipBinding", "none"));
        }
        c.stolenMaterialId = extractStringValue(object, "stolenMaterialId", "");
        c.stolenMaterialName = extractStringValue(object, "stolenMaterialName", "");
        c.stolenMaterialQuality = extractStringValue(object, "stolenMaterialQuality", "normal");
        c.stolenMaterialQuantity = std::max(0, extractIntValue(object, "stolenMaterialQuantity", 0));
    }
    for (const std::string& object : vaultTheftCaseWeaponObjects)
    {
        const std::string caseId = extractStringValue(object, "caseId", "");
        if (caseId.empty()) continue;
        PlayerVaultTheftCase& c = findOrCreateTheftCase(caseId);
        const std::string name = extractStringValue(object, "name", "Épée rouillée");
        Weapon fallback = weaponFromName(name);
        Weapon w(name, extractStringValue(object, "description", fallback.getDescription()), extractIntValue(object, "value", fallback.getValue()),
            weaponTypeFromSaveText(extractStringValue(object, "type", weaponTypeToSaveText(fallback.getType()))),
            extractIntValue(object, "minDamageBonus", fallback.getMinDamageBonus()), extractIntValue(object, "maxDamageBonus", fallback.getMaxDamageBonus()),
            extractIntValue(object, "criticalBonus", fallback.getCriticalBonus()), extractIntValue(object, "maxDurability", fallback.getMaxDurability()));
        applySavedWeaponDurability(w, extractIntValue(object, "durability", w.getDurability()));
        w.loadEnchantmentsFromSaveText(extractStringValue(object, "enchantments", ""));
        w.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(w, object);
        w.setPersistentId(extractStringValue(object, "persistentId", ""));
        c.stolenWeapon = w; c.hasWeapon = true;
    }
    for (const std::string& object : vaultTheftCaseArmorObjects)
    {
        const std::string caseId = extractStringValue(object, "caseId", "");
        if (caseId.empty()) continue;
        PlayerVaultTheftCase& c = findOrCreateTheftCase(caseId);
        const std::string name = extractStringValue(object, "name", "Tenue simple");
        Armor fallback = armorFromName(name);
        Armor a(name, extractStringValue(object, "description", fallback.getDescription()), extractIntValue(object, "value", fallback.getValue()),
            armorTypeFromSaveText(extractStringValue(object, "type", armorTypeToSaveText(fallback.getType()))),
            extractIntValue(object, "maxHpBonus", fallback.getMaxHpBonus()), extractIntValue(object, "damageReduction", fallback.getDamageReduction()),
            extractIntValue(object, "maxDurability", fallback.getMaxDurability()));
        applySavedArmorDurability(a, extractIntValue(object, "durability", a.getDurability()));
        a.loadEnchantmentsFromSaveText(extractStringValue(object, "enchantments", ""));
        a.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(a, object);
        a.setPersistentId(extractStringValue(object, "persistentId", ""));
        c.stolenArmor = a; c.hasArmor = true;
    }

    std::vector<std::string> loadedRegisteredGuildCityIds;
    for (const std::string& object : registeredGuildCityObjects)
    {
        const std::string cityId = extractStringValue(object, "id", "");
        if (!cityId.empty()) loadedRegisteredGuildCityIds.push_back(cityId);
    }
    player.setLoadedCityState(
        extractBoolValue(content, "vaultPurchased", false),
        extractIntValue(content, "vaultLevel", 0),
        extractStringValue(content, "currentCityId", "valebrume"),
        loadedCityVault,
        loadedRegisteredGuildCityIds,
        loadedCityVaults,
        extractBoolValue(content, "vaultPlaceOptimized", false),
        extractBoolValue(content, "vaultPreparedStorage", false)
    );
    player.setLoadedVaultTheftCases(loadedVaultTheftCases);
    player.setLoadedBattlefieldRecoveryCaches(loadedBattlefieldCaches);
    player.setLoadedCrimeContactStates(loadedCrimeContacts);
    player.setLoadedBackpackTier(extractIntValue(content, "backpackTier", 0));
    player.setLoadedCityVaultRecallMessengerState(
        extractIntValue(content, "vaultRecallMessengerStamp", -1000000),
        extractStringValue(content, "vaultRecallMessengerSourceCityId", ""),
        extractStringValue(content, "vaultRecallMessengerDestinationCityId", "")
    );

    // V3.50.19 / schema 25: historical economy numbers were authored as small price points
    // but were converted as whole PO. Normalize old wallets exactly once so one historical
    // price/reward point becomes one PF (10 PC), not one PO (1000 PC).
    if (loadedSaveVersion < 25)
    {
        if (totalCopperCurrency >= 0)
        {
            // Convert each legacy whole-PO block to one economy unit (PF) while preserving
            // the sub-PO copper remainder. This keeps already-exact PC micro-payments intact.
            const long long legacyWholeGold = totalCopperCurrency / Money::COPPER_PER_GOLD;
            const long long exactCopperRemainder = totalCopperCurrency % Money::COPPER_PER_GOLD;
            totalCopperCurrency = Money::copperFromEconomyUnits(legacyWholeGold) + exactCopperRemainder;
        }
        else
        {
            totalCopperCurrency = Money::copperFromEconomyUnits(gold);
        }
    }

    // Schema 26 preserves the physical denomination stacks. Older saves only know a
    // total value, so they are decomposed once during migration; no value is lost.
    const bool hasSavedCoinStacks = loadedSaveVersion >= 26
        && savedCopperCoins >= 0
        && savedIronCoins >= 0
        && savedElectrumCoins >= 0
        && savedGoldCoins >= 0
        && savedPlatinumCoins >= 0;

    if (hasSavedCoinStacks)
    {
        CoinBreakdown savedStacks;
        savedStacks.copper = savedCopperCoins;
        savedStacks.iron = savedIronCoins;
        savedStacks.electrum = savedElectrumCoins;
        savedStacks.gold = savedGoldCoins;
        savedStacks.platinum = savedPlatinumCoins;
        player.getInventory().setCoinStacks(savedStacks);
    }
    else if (totalCopperCurrency >= 0)
    {
        player.getInventory().setTotalCopper(totalCopperCurrency);
    }
    else
    {
        player.getInventory().setGold(gold);
    }
    player.setLoadedProgress(level, experience, hp);

    DndAttributes loadedAttributes;
    loadedAttributes.setValues(strength, dexterity, constitution, intelligence, wisdom, charisma);
    player.setLoadedAttributes(loadedAttributes, unspentAttributePoints);
    player.setLoadedManaState(loadedMana < 0 ? player.getMaxMana() : loadedMana, loadedManaExhaustionTurns);
    player.setLoadedStatistics(
        combatsStarted,
        worldDaysElapsed,
        worldDayProgressUnits,
        victories,
        defeats,
        escapes,
        deaths,
        enemiesKilled,
        bossesKilled,
        pvpVictories,
        pvpDefeats
    );
    player.setLocalSubscriptionRenewalPaidThisWeek(localSubscriptionRenewalPaidThisWeek);

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

    std::vector<PlayerLocalSubscription> loadedLocalSubscriptions;
    for (const std::string& object : localSubscriptionObjects)
    {
        PlayerLocalSubscription subscription;
        subscription.id = extractStringValue(object, "id", "");
        subscription.name = extractStringValue(object, "name", subscription.id);
        subscription.expiresAtDay = extractIntValue(object, "expiresAtDay", -1);
        subscription.cancellationRequested = extractBoolValue(object, "cancellationRequested", false);
        subscription.renewalPrice = extractIntValue(object, "renewalPrice", 0);
        if (!subscription.id.empty())
        {
            loadedLocalSubscriptions.push_back(subscription);
        }
    }
    player.setLoadedLocalSubscriptions(loadedLocalSubscriptions);

    std::vector<PlayerCurse> loadedCurses;
    for (const std::string& object : activeCurseObjects)
    {
        PlayerCurse curse;
        curse.id = extractStringValue(object, "id", "");
        curse.name = extractStringValue(object, "name", curse.id);
        curse.severity = extractStringValue(object, "severity", "");
        curse.origin = extractStringValue(object, "origin", "");
        curse.description = extractStringValue(object, "description", "");
        curse.removalHint = extractStringValue(object, "removalHint", "");
        curse.symptomCategories = extractStringValue(object, "symptomCategories", "");
        curse.discoveredSymptomCategories = extractStringValue(object, "discoveredSymptomCategories", "");
        curse.excludedSymptomCategories = extractStringValue(object, "excludedSymptomCategories", "");
        curse.diagnosisLevel = extractIntValue(object, "diagnosisLevel", 0);
        curse.appliedAtDay = extractIntValue(object, "appliedAtDay", 0);
        curse.expiresAtDay = extractIntValue(object, "expiresAtDay", -1);
        curse.exorcismProgress = extractIntValue(object, "exorcismProgress", 0);
        curse.exorcismRequiredVisits = extractIntValue(object, "exorcismRequiredVisits", 0);
        curse.curseLevel = extractIntValue(object, "curseLevel", 1);
        curse.maxCurseLevel = extractIntValue(object, "maxCurseLevel", curse.curseLevel);
        curse.evolvesOverTime = extractBoolValue(object, "evolvesOverTime", false);
        curse.escalationIntervalDays = extractIntValue(object, "escalationIntervalDays", 0);
        curse.nextEscalationDay = extractIntValue(object, "nextEscalationDay", -1);
        curse.churchRemovalMaxLevel = extractIntValue(object, "churchRemovalMaxLevel", 99);
        curse.becomesSpecialRemovalWhenTooHigh = extractBoolValue(object, "becomesSpecialRemovalWhenTooHigh", false);
        curse.highLevelRemovalHint = extractStringValue(object, "highLevelRemovalHint", "");
        curse.removableByChurch = extractBoolValue(object, "removableByChurch", false);
        curse.bossIdRequiredToBreak = extractIntValue(object, "bossIdRequiredToBreak", 0);
        curse.lifeLong = extractBoolValue(object, "lifeLong", false);
        if (!curse.id.empty())
        {
            loadedCurses.push_back(curse);
        }
    }
    player.setLoadedCurses(loadedCurses);

    std::vector<Blessing> loadedBlessings;
    for (const std::string& object : activeBlessingObjects)
    {
        Blessing blessing(
            extractStringValue(object, "id", ""),
            extractStringValue(object, "name", ""),
            extractStringValue(object, "description", ""),
            extractBoolValue(object, "survivalProtection", false)
        );
        if (blessing.isValid())
        {
            loadedBlessings.push_back(blessing);
        }
    }
    player.setLoadedBlessings(loadedBlessings);

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

    std::vector<std::string> loadedEnabledPassiveSkills;
    for (const std::string& object : enabledPassiveSkillObjects)
    {
        std::string id = extractStringValue(object, "id", "");
        if (!id.empty())
        {
            loadedEnabledPassiveSkills.push_back(id);
        }
    }

    std::vector<std::string> loadedEquippedActiveSkills;
    for (const std::string& object : equippedActiveSkillObjects)
    {
        std::string id = extractStringValue(object, "id", "");
        if (!id.empty())
        {
            loadedEquippedActiveSkills.push_back(id);
        }
    }

    player.setLoadedSkillState(
        loadedPassiveSkills,
        loadedActiveSkills,
        extractIntValue(content, "daggerKillProgress", 0),
        extractIntValue(content, "bowKillProgress", 0),
        extractIntValue(content, "bareHandKillProgress", 0),
        extractIntValue(content, "staffKillProgress", 0),
        extractIntValue(content, "swordKillProgress", 0),
        extractIntValue(content, "axeKillProgress", 0),
        extractIntValue(content, "hammerKillProgress", 0),
        extractIntValue(content, "spearKillProgress", 0)
    );
    player.setLoadedSkillLoadout(loadedEnabledPassiveSkills, loadedEquippedActiveSkills);

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

    std::vector<int> loadedDefeatedBossIds;
    for (const std::string& object : defeatedBossObjects)
    {
        int id = extractIntValue(object, "id", 0);
        if (id > 0)
        {
            loadedDefeatedBossIds.push_back(id);
        }
    }

    player.setLoadedBossRegistry(loadedUnlockedBossIds, loadedRecentBossIds, loadedDefeatedBossIds, recentBossCooldownExpiresAtDay, rareBossDiscoveryCooldownExpiresAtDay);

    std::vector<std::string> loadedBossDiscoveryLocations(37, "");
    for (const std::string& object : bossDiscoveryLocationObjects)
    {
        const int id = extractIntValue(object, "id", 0);
        if (id > 0 && id < static_cast<int>(loadedBossDiscoveryLocations.size()))
        {
            loadedBossDiscoveryLocations[static_cast<std::size_t>(id)] = extractStringValue(object, "location", "");
        }
    }
    player.setLoadedBossDiscoveryLocations(loadedBossDiscoveryLocations);

    std::vector<std::string> loadedRecentExplorationEvents;
    for (const std::string& object : recentExplorationEventObjects)
    {
        const std::string id = extractStringValue(object, "id", "");
        if (!id.empty()) loadedRecentExplorationEvents.push_back(id);
    }
    std::vector<std::string> loadedRecentExplorationChallenges;
    for (const std::string& object : recentExplorationChallengeObjects)
    {
        const std::string id = extractStringValue(object, "id", "");
        if (!id.empty()) loadedRecentExplorationChallenges.push_back(id);
    }
    player.setLoadedExplorationVarietyHistory(loadedRecentExplorationEvents, loadedRecentExplorationChallenges);

    std::vector<PlayerExplorationCooldown> loadedExplorationSceneCooldowns;
    for (const std::string& object : explorationSceneCooldownObjects)
    {
        PlayerExplorationCooldown cooldown;
        cooldown.key = extractStringValue(object, "key", "");
        cooldown.expiresAtDay = extractIntValue(object, "expiresAtDay", -1);
        if (!cooldown.key.empty() && cooldown.expiresAtDay > player.getWorldDaysElapsed())
        {
            loadedExplorationSceneCooldowns.push_back(cooldown);
        }
    }
    player.setLoadedExplorationSceneCooldowns(loadedExplorationSceneCooldowns);

    std::vector<PlayerPersistentCounter> loadedShopPromotionPurchaseCounters;
    for (const std::string& object : shopPromotionPurchaseObjects)
    {
        PlayerPersistentCounter counter;
        counter.key = extractStringValue(object, "key", "");
        counter.value = extractIntValue(object, "value", 0);
        if (!counter.key.empty() && counter.value > 0)
        {
            loadedShopPromotionPurchaseCounters.push_back(counter);
        }
    }
    player.setLoadedShopPromotionPurchaseCounters(loadedShopPromotionPurchaseCounters);

    std::vector<PlayerJournalRecord> loadedCanonicalJournalRecords;
    for (const std::string& object : canonicalJournalObjects)
    {
        PlayerJournalRecord record;
        record.category = extractStringValue(object, "category", "");
        record.key = extractStringValue(object, "key", "");
        record.label = extractStringValue(object, "label", record.key);
        record.locationId = extractStringValue(object, "locationId", "");
        record.count = extractIntValue(object, "count", 0);
        record.lastDay = extractIntValue(object, "lastDay", 0);
        if (!record.category.empty() && !record.key.empty() && record.count > 0)
        {
            loadedCanonicalJournalRecords.push_back(record);
        }
    }
    player.setLoadedCanonicalJournalRecords(loadedCanonicalJournalRecords);

    std::vector<PlayerHistoricalEvent> loadedHistoricalEvents;
    for (const std::string& object : historicalEventObjects)
    {
        PlayerHistoricalEvent event;
        event.id = extractStringValue(object, "id", "");
        event.category = extractStringValue(object, "category", "");
        event.subjectId = extractStringValue(object, "subjectId", "");
        event.label = extractStringValue(object, "label", event.subjectId);
        event.locationId = extractStringValue(object, "locationId", "");
        event.day = extractIntValue(object, "day", 0);
        event.resolved = extractBoolValue(object, "resolved", false);
        if (!event.category.empty() && !event.subjectId.empty()) loadedHistoricalEvents.push_back(event);
    }
    player.setLoadedHistoricalEvents(loadedHistoricalEvents);

    std::vector<PlayerRivalRecord> loadedRivals;
    for (const std::string& object : rivalObjects)
    {
        PlayerRivalRecord rival;
        rival.rivalId = extractStringValue(object, "rivalId", "");
        rival.enemyName = extractStringValue(object, "enemyName", "");
        rival.enemyFamily = extractStringValue(object, "enemyFamily", "");
        rival.originLocationId = extractStringValue(object, "originLocationId", "");
        rival.lastKnownLocationId = extractStringValue(object, "lastKnownLocationId", rival.originLocationId);
        rival.rivalryReason = extractStringValue(object, "rivalryReason", "");
        rival.temperament = extractStringValue(object, "temperament", "survivant prudent");
        rival.visibleMark = extractStringValue(object, "visibleMark", "");
        rival.lastOutcome = extractStringValue(object, "lastOutcome", "");
        rival.baseLevel = extractIntValue(object, "baseLevel", 1);
        rival.currentLevel = extractIntValue(object, "currentLevel", rival.baseLevel);
        rival.baseMaxHp = extractIntValue(object, "baseMaxHp", 1);
        rival.baseAttack = extractIntValue(object, "baseAttack", 1);
        rival.encounters = extractIntValue(object, "encounters", 1);
        rival.escapes = extractIntValue(object, "escapes", 0);
        rival.returns = extractIntValue(object, "returns", 0);
        rival.wounds = extractIntValue(object, "wounds", 0);
        rival.emergenceScore = extractIntValue(object, "emergenceScore", 0);
        rival.notoriety = extractIntValue(object, "notoriety", 0);
        rival.firstSeenDay = extractIntValue(object, "firstSeenDay", 0);
        rival.lastSeenDay = extractIntValue(object, "lastSeenDay", 0);
        rival.alive = extractBoolValue(object, "alive", true);
        rival.stolenCopper = std::max<long long>(0, extractLongLongValue(object, "stolenCopper", 0));
        rival.stolenMaterialId = extractStringValue(object, "stolenMaterialId", "");
        rival.stolenMaterialName = extractStringValue(object, "stolenMaterialName", "");
        rival.stolenMaterialQuality = extractStringValue(object, "stolenMaterialQuality", "normal");
        rival.stolenMaterialQuantity = std::max(0, extractIntValue(object, "stolenMaterialQuantity", 0));
        if (!rival.enemyName.empty()) loadedRivals.push_back(rival);
    }
    player.setLoadedRivalRecords(loadedRivals);
    if (loadedHistoricalEvents.empty()) player.migrateImportantCanonicalHistory();

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
        loadedGrinkaWeapon.loadEnchantmentsFromSaveText(extractStringValue(object, "enchantments", ""));
        loadedGrinkaWeapon.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(loadedGrinkaWeapon, object);
        loadedGrinkaWeapon.setPersistentId(extractStringValue(object, "persistentId", ""));
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
        loadedGrinkaArmor.loadEnchantmentsFromSaveText(extractStringValue(object, "enchantments", ""));
        loadedGrinkaArmor.loadOwnershipBindingFromSaveText(extractStringValue(object, "ownershipBinding", "none"));
        loadRecoveryMetadata(loadedGrinkaArmor, object);
        loadedGrinkaArmor.setPersistentId(extractStringValue(object, "persistentId", ""));
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
        record.habitat = extractStringValue(object, "habitat", "");
        record.weaknesses = extractStringValue(object, "weaknesses", "");
        record.resistances = extractStringValue(object, "resistances", "");
        record.drops = extractStringValue(object, "drops", "");
        record.strategy = extractStringValue(object, "strategy", "");
        record.dangerRank = extractStringValue(object, "dangerRank", "");
        record.encounters = extractIntValue(object, "encounters", 0);
        record.kills = extractIntValue(object, "kills", 0);
        record.informationBought = extractBoolValue(object, "informationBought", false);

        BestiaryRuntimeProgress::importRecord(record);
    }

    player.getQuestLog().clear();

    auto loadQuestSnapshotEntry = [](const std::string& object) {
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
        quest.rewardCoins.copper = extractLongLongValue(object, "rewardCoinCopper", 0);
        quest.rewardCoins.iron = extractLongLongValue(object, "rewardCoinIron", 0);
        quest.rewardCoins.electrum = extractLongLongValue(object, "rewardCoinElectrum", 0);
        quest.rewardCoins.gold = extractLongLongValue(object, "rewardCoinGold", 0);
        quest.rewardCoins.platinum = extractLongLongValue(object, "rewardCoinPlatinum", 0);
        quest.rewardMaterialId = extractStringValue(object, "rewardMaterialId", "");
        quest.rewardMaterialName = extractStringValue(object, "rewardMaterialName", "");
        quest.rewardMaterialQuantity = extractIntValue(object, "rewardMaterialQuantity", 0);
        quest.rewardNote = extractStringValue(object, "rewardNote", "");
        quest.requiredMaterialId = extractStringValue(object, "requiredMaterialId", "");
        quest.requiredMaterialName = extractStringValue(object, "requiredMaterialName", "");
        quest.requiredMaterialQuantity = extractIntValue(object, "requiredMaterialQuantity", 0);
        quest.objectiveType = extractStringValue(object, "objectiveType", quest.requiredMaterialId.empty() ? "combat" : "livraison");
        quest.targetFamily = extractStringValue(object, "targetFamily", "Général");
        quest.progress = extractIntValue(object, "progress", 0);
        quest.target = extractIntValue(object, "target", 1);
        quest.availableFromDay = extractIntValue(object, "availableFromDay", 0);
        quest.expiresAtDay = extractIntValue(object, "expiresAtDay", -1);
        quest.guildQuest = extractBoolValue(object, "guildQuest", false);
        quest.guildChallenge = extractBoolValue(object, "guildChallenge", false);
        quest.challengeCondition = extractStringValue(object, "challengeCondition", "");
        quest.challengeMarkReward = extractIntValue(object, "challengeMarkReward", 0);
        quest.accepted = extractBoolValue(object, "accepted", true);
        quest.completed = extractBoolValue(object, "completed", false);
        quest.turnedIn = extractBoolValue(object, "turnedIn", false);
        quest.failed = extractBoolValue(object, "failed", false);
        quest.failureReason = extractStringValue(object, "failureReason", "");
        quest.requiredLanguage = extractStringValue(object, "requiredLanguage", "");
        quest.requiredLanguageLevel = extractIntValue(object, "requiredLanguageLevel", quest.requiredLanguage.empty() ? 0 : 2);
        quest.sourceLanguageText = extractStringValue(object, "sourceLanguageText", "");
        quest.linkedQuestIds = extractStringValue(object, "linkedQuestIds", "");
        quest.stageLabels = extractStringValue(object, "stageLabels", "");
        quest.serviceChallengeHistory = extractStringValue(object, "serviceChallengeHistory", "");
        quest.linkedQuestRequiredState = extractStringValue(object, "linkedQuestRequiredState", "turned_in");
        quest.retroactiveProgress = extractBoolValue(object, "retroactiveProgress", false);
        quest.hideFutureSteps = extractBoolValue(object, "hideFutureSteps", false);
        quest.phaseTwoChecked = extractBoolValue(object, "phaseTwoChecked", false);
        quest.phaseTwoOffered = extractBoolValue(object, "phaseTwoOffered", false);
        quest.phaseTwoActive = extractBoolValue(object, "phaseTwoActive", false);
        quest.phaseTwoMandatory = extractBoolValue(object, "phaseTwoMandatory", false);
        quest.phaseTwoCompleted = extractBoolValue(object, "phaseTwoCompleted", false);
        quest.phaseTwoDeclined = extractBoolValue(object, "phaseTwoDeclined", false);
        quest.phaseTwoTitle = extractStringValue(object, "phaseTwoTitle", "");
        quest.phaseTwoPreviousSummary = extractStringValue(object, "phaseTwoPreviousSummary", "");
        quest.phaseTwoReason = extractStringValue(object, "phaseTwoReason", "");
        quest.phaseTwoObjective = extractStringValue(object, "phaseTwoObjective", "");
        quest.phaseTwoProgress = extractIntValue(object, "phaseTwoProgress", 0);
        quest.phaseTwoTarget = extractIntValue(object, "phaseTwoTarget", 0);
        quest.phaseTwoAttempts = extractIntValue(object, "phaseTwoAttempts", 0);
        quest.phaseTwoRewardExperience = extractIntValue(object, "phaseTwoRewardExperience", 0);
        quest.phaseTwoRewardGold = extractIntValue(object, "phaseTwoRewardGold", 0);
        quest.phaseTwoRewardKind = extractStringValue(object, "phaseTwoRewardKind", "");
        quest.phaseTwoRewardDetail = extractStringValue(object, "phaseTwoRewardDetail", "");
        quest.phaseTwoTravelRequired = extractBoolValue(object, "phaseTwoTravelRequired", false);
        quest.phaseTwoTravelSourceCityId = extractStringValue(object, "phaseTwoTravelSourceCityId", "");
        quest.phaseTwoTravelDestinationCityId = extractStringValue(object, "phaseTwoTravelDestinationCityId", "");
        quest.phaseTwoTravelDistanceKm = extractIntValue(object, "phaseTwoTravelDistanceKm", 0);
        quest.intercityMission = extractBoolValue(object, "intercityMission", false);
        quest.intercityKind = extractStringValue(object, "intercityKind", "");
        quest.intercitySourceCityId = extractStringValue(object, "intercitySourceCityId", "");
        quest.intercityRouteCityIds = extractStringValue(object, "intercityRouteCityIds", "");
        quest.intercityPayloadName = extractStringValue(object, "intercityPayloadName", "");
        quest.intercityStopIndex = extractIntValue(object, "intercityStopIndex", 0);
        quest.intercityTotalDistanceKm = extractIntValue(object, "intercityTotalDistanceKm", 0);
        quest.intercityTravelRewardGold = extractIntValue(object, "intercityTravelRewardGold", 0);
        quest.intercityTravelRewardExperience = extractIntValue(object, "intercityTravelRewardExperience", 0);
        quest.tradeIncidentMission = extractBoolValue(object, "tradeIncidentMission", false);
        quest.tradeIncidentId = extractStringValue(object, "tradeIncidentId", "");
        quest.tradeIncidentKind = extractStringValue(object, "tradeIncidentKind", "");
        quest.tradeIncidentSourceCityId = extractStringValue(object, "tradeIncidentSourceCityId", "");
        quest.tradeIncidentDestinationCityId = extractStringValue(object, "tradeIncidentDestinationCityId", "");
        quest.tradeIncidentGoodsType = extractIntValue(object, "tradeIncidentGoodsType", -1);
        quest.tradeIncidentSeverity = extractIntValue(object, "tradeIncidentSeverity", 0);
        quest.tradeIncidentStage = extractStringValue(object, "tradeIncidentStage", "");
        quest.tradeIncidentAttempts = extractIntValue(object, "tradeIncidentAttempts", 0);
        quest.tradeIncidentCreatesBountyLead = extractBoolValue(object, "tradeIncidentCreatesBountyLead", false);
        quest.tradeIncidentBountyPublished = extractBoolValue(object, "tradeIncidentBountyPublished", false);
        quest.bountyMission = extractBoolValue(object, "bountyMission", false);
        quest.bountyTargetId = extractStringValue(object, "bountyTargetId", "");
        quest.bountyTargetName = extractStringValue(object, "bountyTargetName", "");
        quest.bountyTargetTemperament = extractStringValue(object, "bountyTargetTemperament", "");
        quest.bountyTargetHomeCityId = extractStringValue(object, "bountyTargetHomeCityId", "");
        quest.bountyTargetNetworkId = extractStringValue(object, "bountyTargetNetworkId", "");
        quest.bountyIssuerId = extractStringValue(object, "bountyIssuerId", "");
        quest.bountyIssuerName = extractStringValue(object, "bountyIssuerName", "");
        quest.bountyIssuerOrganizationId = extractStringValue(object, "bountyIssuerOrganizationId", "");
        quest.bountyIssuerOrganizationName = extractStringValue(object, "bountyIssuerOrganizationName", "");
        quest.bountyIssuerPrefersLiveCapture = extractBoolValue(object, "bountyIssuerPrefersLiveCapture", true);
        quest.bountyCloseRelationId = extractStringValue(object, "bountyCloseRelationId", "");
        quest.bountyCloseRelationName = extractStringValue(object, "bountyCloseRelationName", "");
        quest.bountyCloseRelationType = extractStringValue(object, "bountyCloseRelationType", "");
        quest.bountyStage = extractStringValue(object, "bountyStage", quest.bountyMission ? "investigation" : "");
        quest.bountyClues = extractIntValue(object, "bountyClues", 0);
        quest.bountyInvestigationNoise = extractIntValue(object, "bountyInvestigationNoise", 0);
        quest.bountyInformantExposure = extractIntValue(object, "bountyInformantExposure", 0);
        quest.bountyTargetAwareness = extractIntValue(object, "bountyTargetAwareness", 45);
        quest.bountyTargetSuspectsHunt = extractBoolValue(object, "bountyTargetSuspectsHunt", false);
        quest.bountyTargetKnowsHunt = extractBoolValue(object, "bountyTargetKnowsHunt", false);
        quest.bountyTargetResources = extractIntValue(object, "bountyTargetResources", 45);
        quest.bountyTargetLocalSupport = extractIntValue(object, "bountyTargetLocalSupport", 20);
        quest.bountyTargetMobility = extractIntValue(object, "bountyTargetMobility", 50);
        quest.bountyTargetEscapeAttempts = extractIntValue(object, "bountyTargetEscapeAttempts", 0);
        quest.bountyTargetKnowsTerrain = extractBoolValue(object, "bountyTargetKnowsTerrain", false);
        quest.bountyTargetInjured = extractBoolValue(object, "bountyTargetInjured", false);
        quest.bountyExitRecon = extractIntValue(object, "bountyExitRecon", 0);
        quest.bountyInformantNetwork = extractIntValue(object, "bountyInformantNetwork", 0);
        quest.bountyAmbushPreparation = extractIntValue(object, "bountyAmbushPreparation", 0);
        quest.bountyShadowingPreparation = extractIntValue(object, "bountyShadowingPreparation", 0);
        quest.bountyRouteVariation = extractIntValue(object, "bountyRouteVariation", 0);
        quest.bountyDecoyQuality = extractIntValue(object, "bountyDecoyQuality", 0);
        quest.bountyCounterSurveillance = extractIntValue(object, "bountyCounterSurveillance", 0);
        quest.bountyRefugeConcealment = extractIntValue(object, "bountyRefugeConcealment", 0);
        quest.bountyDangerRating = extractIntValue(object, "bountyDangerRating", 1);
        quest.bountyNotorietyRating = extractIntValue(object, "bountyNotorietyRating", 1);
        quest.bountyDaysAtLarge = extractIntValue(object, "bountyDaysAtLarge", 0);
        quest.bountyBaseRewardGold = extractIntValue(object, "bountyBaseRewardGold", 0);
        quest.bountyRewardProfileApplied = extractBoolValue(object, "bountyRewardProfileApplied", false);
        quest.bountyRegionalProfileApplied = extractBoolValue(object, "bountyRegionalProfileApplied", false);
        quest.bountyCurrentCityId = extractStringValue(object, "bountyCurrentCityId", "");
        quest.bountyCurrentBiome = extractStringValue(object, "bountyCurrentBiome", "");
        quest.bountyVisitedCityIds = extractStringValue(object, "bountyVisitedCityIds", "");
        quest.bountyRelocationCount = extractIntValue(object, "bountyRelocationCount", 0);
        quest.bountyTransitTrailKind = extractStringValue(object, "bountyTransitTrailKind", "");
        quest.bountyTransitTrailFromCityId = extractStringValue(object, "bountyTransitTrailFromCityId", "");
        quest.bountyTransitTrailReliability = extractIntValue(object, "bountyTransitTrailReliability", 0);
        quest.bountyTransitTrailAvailable = extractBoolValue(object, "bountyTransitTrailAvailable", false);
        quest.bountyRegionalProfileCityId = extractStringValue(object, "bountyRegionalProfileCityId", "");
        quest.bountyRegionalConsequencesApplied = extractBoolValue(object, "bountyRegionalConsequencesApplied", false);
        quest.bountyResolution = extractStringValue(object, "bountyResolution", "");
        if (quest.failed && quest.failureReason.empty())
        {
            quest.failureReason = "Délai dépassé ou demande archivée sans validation.";
        }
        return quest;
    };

    for (const std::string& object : questObjects)
    {
        player.getQuestLog().getQuests().push_back(loadQuestSnapshotEntry(object));
    }
    player.getQuestLog().refreshLinkedQuestProgress();

    std::vector<Quest> loadedGuildBoardOffers;
    for (const std::string& object : guildBoardObjects)
    {
        loadedGuildBoardOffers.push_back(loadQuestSnapshotEntry(object));
    }

    player.getQuestLog().setLoadedGuildBoardState(
        loadedGuildBoardOffers,
        extractIntValue(content, "createdAtCombat", -1),
        extractIntValue(content, "targetSize", static_cast<int>(loadedGuildBoardOffers.size())),
        extractIntValue(content, "pendingReplacements", 0),
        extractIntValue(content, "replacementDueAtCombat", -1)
    );

    std::vector<Quest> loadedGuildChallengeBoardOffers;
    for (const std::string& object : guildChallengeBoardObjects)
    {
        loadedGuildChallengeBoardOffers.push_back(loadQuestSnapshotEntry(object));
    }
    player.getQuestLog().setLoadedGuildChallengeBoardState(
        loadedGuildChallengeBoardOffers,
        extractIntValue(content, "guildChallengeCreatedAtDay", -1)
    );

    return true;
}



bool SaveManager::createImportantUpdateBackup(
    const CharacterSaveSummary& summary,
    std::string& backupDirectory
)
{
    backupDirectory.clear();

    try
    {
        if (summary.path.empty() || !std::filesystem::exists(summary.path))
        {
            return false;
        }

        const std::string checkpointVersion = VersionInfo::importantSaveUpdateVersion();
        const std::string safeAccount = buildSafeFileName(summary.accountName);
        const std::string safeCharacter = buildSafeFileName(summary.characterName);
        const std::string root = SAVE_ROOT
            + "/update_backups/V" + checkpointVersion
            + "/" + safeAccount;

        std::filesystem::create_directories(root);
        backupDirectory = root;

        const std::string characterBackup = root + "/" + safeCharacter + "__before_V" + checkpointVersion + ".json";
        if (!std::filesystem::exists(characterBackup))
        {
            std::filesystem::copy_file(summary.path, characterBackup, std::filesystem::copy_options::none);
        }

        const std::string accountSource = getAccountSavePath(summary.accountName);
        const std::string accountBackup = root + "/account__before_V" + checkpointVersion + ".json";
        if (std::filesystem::exists(accountSource) && !std::filesystem::exists(accountBackup))
        {
            std::filesystem::copy_file(accountSource, accountBackup, std::filesystem::copy_options::none);
        }

        const std::string manifestPath = root + "/checkpoint.txt";
        if (!std::filesystem::exists(manifestPath))
        {
            std::ofstream manifest(manifestPath);
            if (manifest.is_open())
            {
                manifest << "Dinotofu important save checkpoint\n";
                manifest << "targetVersion=V" << checkpointVersion << "\n";
                manifest << "character=" << summary.characterName << "\n";
                manifest << "account=" << summary.accountName << "\n";
                manifest << "lastAdaptedVersion=V" << summary.lastAdaptedVersion << "\n";
                manifest << "source=" << summary.path << "\n";
                manifest << "policy=pre_update_backup_never_overwritten\n";
            }
        }

        return std::filesystem::exists(characterBackup);
    }
    catch (...)
    {
        backupDirectory.clear();
        return false;
    }
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


bool SaveManager::transferCharacterOwnership(const CharacterSaveSummary& summary, const std::string& targetAccountName)
{
    if (!ensureSaveDirectories())
    {
        return false;
    }

    if (targetAccountName.empty() || summary.path.empty() || !std::filesystem::exists(summary.path))
    {
        return false;
    }

    if (buildSafeFileName(targetAccountName) == buildSafeFileName(summary.accountName))
    {
        return false;
    }

    std::string safeTargetAccount = buildSafeFileName(targetAccountName);
    std::string safeCharacter = buildSafeFileName(summary.characterName);
    std::string destination = SAVE_ROOT + "/characters/playable/" + safeTargetAccount + "__" + safeCharacter + ".json";

    if (std::filesystem::exists(destination))
    {
        return false;
    }

    try
    {
        saveAccountSnapshot(targetAccountName);

        std::string content = readFileContent(summary.path);
        if (content.empty())
        {
            return false;
        }

        std::string creator = extractStringValue(content, "creatorAccount", summary.creatorAccountName.empty() ? summary.accountName : summary.creatorAccountName);
        if (creator.empty())
        {
            creator = summary.accountName;
        }

        content = withJsonStringField(content, "account", targetAccountName, false);
        content = withJsonStringField(content, "creatorAccount", creator, false);
        content = withJsonStringField(content, "currentOwnerAccount", targetAccountName, false);
        content = withJsonStringField(content, "creatorAccount", creator, true);
        content = withJsonStringField(content, "currentOwnerAccount", targetAccountName, true);
        content = withJsonStringField(content, "lastTransferNote", "Maitrise transferee de maniere irreversible vers " + targetAccountName, true);

        if (!writeFileContent(destination, content))
        {
            return false;
        }

        std::filesystem::remove(summary.path);
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
