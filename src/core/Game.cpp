// EN: Game.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Game.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "core/Game.hpp"
#include "core/Console.hpp"
#include "core/Random.hpp"
#include "core/VersionInfo.hpp"
#include "class_system/ClassCatalog.hpp"
#include "combat/Combat.hpp"
#include "character/RaceCatalog.hpp"
#include "character/SpecialCharacterNativeBonus.hpp"
#include "save/SaveManager.hpp"
#include "save/menu/AccountMenu.hpp"
#include "save/menu/CharacterMenu.hpp"
#include "economy/shop/ShopRotationSystem.hpp"
#include "economy/shop/ShopTransactionSystem.hpp"
#include "interface/menu/shop/ShopMenu.hpp"
#include "interface/menu/progression/AttributeMenu.hpp"
#include "interface/menu/progression/StatisticsMenu.hpp"
#include "interface/menu/quest/QuestMenu.hpp"
#include "interface/menu/InventoryMenu.hpp"
#include "interface/menu/PostCombatMenu.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "cheat/CheatManager.hpp"
#include "progression/DifficultyRules.hpp"
#include "progression/death/DeathPenaltySystem.hpp"
#include "item/weapon/Weapon.hpp"
#include "item/armor/Armor.hpp"
#include "item/consumable/Consumable.hpp"
#include "item/material/Material.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <set>


namespace
{
    std::string normalizeValueName(std::string value)
    {
        std::string out;
        for (unsigned char character : value)
        {
            if (std::isalnum(character))
            {
                out += static_cast<char>(std::tolower(character));
            }
        }
        return out;
    }

    // EN: valueNameContainsAny declares or implements a focused behavior used by this module.
    // FR: valueNameContainsAny déclare ou implémente un comportement précis utilisé par ce module.
    bool valueNameContainsAny(const std::string& value, const std::vector<std::string>& words)
    {
        std::string normalized = normalizeValueName(value);
        for (const std::string& word : words)
        {
            if (normalized.find(normalizeValueName(word)) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    // EN: rarityEstimateMultiplier declares or implements a focused behavior used by this module.
    // FR: rarityEstimateMultiplier déclare ou implémente un comportement précis utilisé par ce module.
    int rarityEstimateMultiplier(const std::string& name)
    {
        if (valueNameContainsAny(name, {"Relique", "Unique", "Divin", "God"})) return 5;
        if (valueNameContainsAny(name, {"Héroïque", "Heroique", "Légendaire", "Legendaire"})) return 3;
        if (valueNameContainsAny(name, {"Rare", "Mystique"})) return 2;
        return 1;
    }

    // EN: estimateWeaponTradeValue declares or implements a focused behavior used by this module.
    // FR: estimateWeaponTradeValue déclare ou implémente un comportement précis utilisé par ce module.
    int estimateWeaponTradeValue(const Weapon& weapon)
    {
        int value = weapon.getValue();
        value += weapon.getMinDamageBonus() * 8;
        value += weapon.getMaxDamageBonus() * 10;
        value += weapon.getCriticalBonus() * 6;
        value *= rarityEstimateMultiplier(weapon.getName());

        if (!weapon.isIndestructible() && weapon.getMaxDurability() > 0)
        {
            value = value * std::max(1, weapon.getDurability()) / weapon.getMaxDurability();
        }

        return std::max(1, value);
    }

    // EN: estimateArmorTradeValue declares or implements a focused behavior used by this module.
    // FR: estimateArmorTradeValue déclare ou implémente un comportement précis utilisé par ce module.
    int estimateArmorTradeValue(const Armor& armor)
    {
        int value = armor.getValue();
        value += armor.getMaxHpBonus() * 8;
        value *= rarityEstimateMultiplier(armor.getName());

        if (!armor.isIndestructible() && armor.getMaxDurability() > 0)
        {
            value = value * std::max(1, armor.getDurability()) / armor.getMaxDurability();
        }

        return std::max(1, value);
    }

    // EN: estimatePlayerTradeValue declares or implements a focused behavior used by this module.
    // FR: estimatePlayerTradeValue déclare ou implémente un comportement précis utilisé par ce module.
    int estimatePlayerTradeValue(const Player& player)
    {
        int total = player.getInventory().getGold();

        for (const Weapon& weapon : player.getInventory().getWeapons())
        {
            total += estimateWeaponTradeValue(weapon);
        }

        for (const Armor& armor : player.getInventory().getArmors())
        {
            total += estimateArmorTradeValue(armor);
        }

        for (const Consumable& consumable : player.getInventory().getConsumables())
        {
            total += consumable.getValue();
        }

        for (const Material& material : player.getInventory().getMaterials())
        {
            total += std::max(1, material.getValue() * material.getQualityPricePercent() / 100) * material.getQuantity();
        }

        return total;
    }

    // EN: displayExchangeValueEstimation declares or implements a focused behavior used by this module.
    // FR: displayExchangeValueEstimation déclare ou implémente un comportement précis utilisé par ce module.
    void displayExchangeValueEstimation(const Player& first, const Player& second)
    {
        MessageScreen::show(
            "ESTIMATION D'ÉCHANGE",
            "exchange.value_estimation",
            {
                "Valeur estimée de " + first.getName() + " : " + std::to_string(estimatePlayerTradeValue(first)) + " pièces.",
                "Valeur estimée de " + second.getName() + " : " + std::to_string(estimatePlayerTradeValue(second)) + " pièces."
            },
            false
        );
    }

    int askExchangeWeaponIndex(const Player& giver)
    {
        const std::vector<Weapon>& weapons = giver.getInventory().getWeapons();

        if (weapons.empty())
        {
            MenuScreen screen("ARME À TRANSFÉRER", "exchange.weapon.empty");
            screen.addLine(giver.getName() + " n'a aucune arme transférable dans son sac.");
            TerminalInterface::renderMenuScreen(screen, false);
            return -1;
        }

        constexpr std::size_t itemsPerPage = 8;
        std::size_t page = 0;

        while (true)
        {
            const std::size_t totalPages = PagedMenu::pageCount(weapons.size(), itemsPerPage);
            const std::size_t first = PagedMenu::firstIndex(page, itemsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(weapons.size(), page, itemsPerPage);

            MenuScreen screen("ARME À TRANSFÉRER", "exchange.weapon.select");
            screen.addSubtitle("Source : " + giver.getName());
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, weapons.size()));

            for (std::size_t i = first; i < last; ++i)
            {
                const Weapon& weapon = weapons[i];
                const bool equipped = static_cast<int>(i) == giver.getEquippedWeaponIndex();
                std::ostringstream hint;
                hint << "Dégâts +" << weapon.getMinDamageBonus() << "/+" << weapon.getMaxDamageBonus()
                     << " | Critique +" << weapon.getCriticalBonus();

                if (weapon.isIndestructible())
                {
                    hint << " | Durabilité : indestructible";
                }
                else
                {
                    hint << " | Durabilité " << weapon.getDurability() << "/" << weapon.getMaxDurability();
                }

                if (equipped)
                {
                    hint << " | équipée";
                }

                screen.addOption(
                    static_cast<int>(i - first + 1),
                    weapon.getName(),
                    hint.str(),
                    !equipped,
                    "exchange.weapon.select"
                );
            }

            PagedMenu::addNavigationOptions(screen, page, totalPages);

            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une arme affichée.");

            if (choice == 0)
            {
                return -1;
            }

            if (choice == 98 && page > 0)
            {
                --page;
                Console::clear();
                continue;
            }

            if (choice == 99 && page + 1 < totalPages)
            {
                ++page;
                Console::clear();
                continue;
            }

            const int visibleCount = static_cast<int>(last - first);
            if (choice >= 1 && choice <= visibleCount)
            {
                return static_cast<int>(first + static_cast<std::size_t>(choice - 1));
            }
        }
    }

    int askExchangeArmorIndex(const Player& giver)
    {
        const std::vector<Armor>& armors = giver.getInventory().getArmors();

        if (armors.empty())
        {
            MenuScreen screen("ARMURE À TRANSFÉRER", "exchange.armor.empty");
            screen.addLine(giver.getName() + " n'a aucune armure transférable dans son sac.");
            TerminalInterface::renderMenuScreen(screen, false);
            return -1;
        }

        constexpr std::size_t itemsPerPage = 8;
        std::size_t page = 0;

        while (true)
        {
            const std::size_t totalPages = PagedMenu::pageCount(armors.size(), itemsPerPage);
            const std::size_t first = PagedMenu::firstIndex(page, itemsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(armors.size(), page, itemsPerPage);

            MenuScreen screen("ARMURE À TRANSFÉRER", "exchange.armor.select");
            screen.addSubtitle("Source : " + giver.getName());
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, armors.size()));

            for (std::size_t i = first; i < last; ++i)
            {
                const Armor& armor = armors[i];
                const bool equipped = static_cast<int>(i) == giver.getEquippedArmorIndex();
                std::ostringstream hint;
                hint << "PV +" << armor.getMaxHpBonus() << " | Réduction " << armor.getDamageReduction();

                if (armor.isIndestructible())
                {
                    hint << " | Durabilité : indestructible";
                }
                else
                {
                    hint << " | Durabilité " << armor.getDurability() << "/" << armor.getMaxDurability();
                }

                if (equipped)
                {
                    hint << " | portée";
                }

                screen.addOption(
                    static_cast<int>(i - first + 1),
                    armor.getName(),
                    hint.str(),
                    !equipped,
                    "exchange.armor.select"
                );
            }

            PagedMenu::addNavigationOptions(screen, page, totalPages);

            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une armure affichée.");

            if (choice == 0)
            {
                return -1;
            }

            if (choice == 98 && page > 0)
            {
                --page;
                Console::clear();
                continue;
            }

            if (choice == 99 && page + 1 < totalPages)
            {
                ++page;
                Console::clear();
                continue;
            }

            const int visibleCount = static_cast<int>(last - first);
            if (choice >= 1 && choice <= visibleCount)
            {
                return static_cast<int>(first + static_cast<std::size_t>(choice - 1));
            }
        }
    }

    int askExchangeConsumableIndex(const Player& giver)
    {
        const std::vector<Consumable>& consumables = giver.getInventory().getConsumables();

        if (consumables.empty())
        {
            MenuScreen screen("CONSOMMABLE À TRANSFÉRER", "exchange.consumable.empty");
            screen.addLine(giver.getName() + " n'a aucun consommable dans son sac.");
            TerminalInterface::renderMenuScreen(screen, false);
            return -1;
        }

        constexpr std::size_t itemsPerPage = 8;
        std::size_t page = 0;

        while (true)
        {
            const std::size_t totalPages = PagedMenu::pageCount(consumables.size(), itemsPerPage);
            const std::size_t first = PagedMenu::firstIndex(page, itemsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(consumables.size(), page, itemsPerPage);

            MenuScreen screen("CONSOMMABLE À TRANSFÉRER", "exchange.consumable.select");
            screen.addSubtitle("Source : " + giver.getName());
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, consumables.size()));

            for (std::size_t i = first; i < last; ++i)
            {
                const Consumable& consumable = consumables[i];
                screen.addOption(
                    static_cast<int>(i - first + 1),
                    consumable.getName(),
                    "Puissance " + std::to_string(consumable.getPower()) + " | Valeur " + std::to_string(consumable.getValue()),
                    true,
                    "exchange.consumable.select"
                );
            }

            PagedMenu::addNavigationOptions(screen, page, totalPages);

            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis un consommable affiché.");

            if (choice == 0)
            {
                return -1;
            }

            if (choice == 98 && page > 0)
            {
                --page;
                Console::clear();
                continue;
            }

            if (choice == 99 && page + 1 < totalPages)
            {
                ++page;
                Console::clear();
                continue;
            }

            const int visibleCount = static_cast<int>(last - first);
            if (choice >= 1 && choice <= visibleCount)
            {
                return static_cast<int>(first + static_cast<std::size_t>(choice - 1));
            }
        }
    }

    int askExchangeMaterialIndex(const Player& giver)
    {
        const std::vector<Material>& materials = giver.getInventory().getMaterials();

        if (materials.empty())
        {
            MenuScreen screen("MATÉRIAU À TRANSFÉRER", "exchange.material.empty");
            screen.addLine(giver.getName() + " n'a aucun matériau dans son sac.");
            TerminalInterface::renderMenuScreen(screen, false);
            return -1;
        }

        constexpr std::size_t itemsPerPage = 8;
        std::size_t page = 0;

        while (true)
        {
            const std::size_t totalPages = PagedMenu::pageCount(materials.size(), itemsPerPage);
            const std::size_t first = PagedMenu::firstIndex(page, itemsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(materials.size(), page, itemsPerPage);

            MenuScreen screen("MATÉRIAU À TRANSFÉRER", "exchange.material.select");
            screen.addSubtitle("Source : " + giver.getName());
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, materials.size()));

            for (std::size_t i = first; i < last; ++i)
            {
                const Material& material = materials[i];
                screen.addOption(
                    static_cast<int>(i - first + 1),
                    material.getName() + " x" + std::to_string(material.getQuantity()),
                    material.getCategory() + " | Qualité " + material.getQualityLabel() + " | Valeur " + std::to_string(material.getValue()),
                    true,
                    "exchange.material.select"
                );
            }

            PagedMenu::addNavigationOptions(screen, page, totalPages);

            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis un matériau affiché.");

            if (choice == 0)
            {
                return -1;
            }

            if (choice == 98 && page > 0)
            {
                --page;
                Console::clear();
                continue;
            }

            if (choice == 99 && page + 1 < totalPages)
            {
                ++page;
                Console::clear();
                continue;
            }

            const int visibleCount = static_cast<int>(last - first);
            if (choice >= 1 && choice <= visibleCount)
            {
                return static_cast<int>(first + static_cast<std::size_t>(choice - 1));
            }
        }
    }
}

// EN: Game declares or implements a focused behavior used by this module.
// FR: Game déclare ou implémente un comportement précis utilisé par ce module.
Game::Game()
{
    accountName = "local";
    playerName = "";
    selectedMode = GameMode::AIPvp;
    selectedDifficulty = DifficultyMode::Normal;
    selectedRace = CharacterRace::Human;
    characterLoaded = false;
    specialIdentityValidated = false;
}

// EN: run declares or implements a focused behavior used by this module.
// FR: run déclare ou implémente un comportement précis utilisé par ce module.
void Game::run()
{
    Console::clear();

    displayIntroduction();
    askAccountName();
    askPlayerName();

    if (!characterLoaded)
    {
        chooseDifficulty();

        if (!specialIdentityValidated)
        {
            choosePlayerRace();
        }

        choosePlayerClass();
    }

    configurePartyMode();
    chooseGameMode();
    displaySelectedMode();
    launchSelectedMode();
}

// EN: displayIntroduction declares or implements a focused behavior used by this module.
// FR: displayIntroduction déclare ou implémente un comportement précis utilisé par ce module.
void Game::displayIntroduction()
{
    MessageScreen::show(
        "DINOTOFU V" + VersionInfo::currentVersion(),
        "game.introduction",
        {
            "Bonjour voyageur, et bienvenue dans Dinotofu.",
            "Un monde de fantaisie, d'arènes et de baston,",
            "où chaque choix peut transformer un simple combattant en légende."
        }
    );
}


// EN: askAccountName declares or implements a focused behavior used by this module.
// FR: askAccountName déclare ou implémente un comportement précis utilisé par ce module.
void Game::askAccountName()
{
    accountName = AccountMenu::open();
}

// EN: askPlayerName declares or implements a focused behavior used by this module.
// FR: askPlayerName déclare ou implémente un comportement précis utilisé par ce module.
void Game::askPlayerName()
{
    CharacterMenuResult result = CharacterMenu::open(accountName, mainPlayer);

    characterLoaded = result.characterLoaded;
    specialIdentityValidated = result.specialIdentityValidated;
    playerName = result.playerName;

    if (result.specialIdentityValidated)
    {
        selectedRace = result.forcedRace;
    }

    if (characterLoaded)
    {
        selectedDifficulty = result.difficulty;
        selectedRace = mainPlayer.getRace();
    }
}

// EN: chooseDifficulty declares or implements a focused behavior used by this module.
// FR: chooseDifficulty déclare ou implémente un comportement précis utilisé par ce module.
void Game::chooseDifficulty()
{
    MenuScreen screen("DIFFICULTÉ", "character.creation.difficulty");
    screen.addSubtitle("Ce choix influence le kit de départ, les récompenses, la mort et le respawn.");
    screen.addOption(1, "Facile", "Plus d'or, plus de sécurité, retour à 75% PV après une mort non définitive.", true, "difficulty.easy");
    screen.addOption(2, "Normal", "L'expérience Dinotofu standard.", true, "difficulty.normal");
    screen.addOption(3, "Difficile", "Moins de ressources, pénalités plus dures, retour à 30% PV.", true, "difficulty.hard");
    screen.addOption(4, "Cauchemar", "Très punitif, retour à 10% PV, et la mort commence vraiment à avoir des dents.", true, "difficulty.nightmare");
    screen.addOption(5, "Léthal", "Le registre ne pardonne pas : une vraie chute peut effacer ton nom.", true, "difficulty.lethal");

    int choice = TerminalInterface::askMenuChoiceFromOptions(
        screen,
        "Veuillez entrer un chiffre correspondant à une difficulté affichée."
    );

    switch (choice)
    {
        case 1:
            selectedDifficulty = DifficultyMode::Easy;
            break;

        case 3:
            selectedDifficulty = DifficultyMode::Hard;
            break;

        case 4:
            selectedDifficulty = DifficultyMode::Nightmare;
            break;

        case 5:
            selectedDifficulty = DifficultyMode::Lethal;
            break;

        case 2:
        default:
            selectedDifficulty = DifficultyMode::Normal;
            break;
    }

    Console::clear();

    MenuScreen confirmation("DIFFICULTÉ VALIDÉE", "character.creation.difficulty.confirmation");
    confirmation.addLine("Difficulté sélectionnée : " + getDifficultyName() + ".");
    confirmation.addLine("Ton départ sera ajusté en conséquence.");

    if (specialIdentityValidated)
    {
        confirmation.addLine("Identité spéciale reconnue : le choix de race est verrouillé par son histoire.");
        confirmation.addLine("Race imposée : " + characterRaceToText(selectedRace) + ".");
    }

    TerminalInterface::renderMenuScreen(confirmation, false);
    Console::waitForEnter();
    Console::clear();
}

// EN: choosePlayerRace declares or implements a focused behavior used by this module.
// FR: choosePlayerRace déclare ou implémente un comportement précis utilisé par ce module.
void Game::choosePlayerRace()
{
    MenuScreen screen("RACE", "character.creation.race");
    screen.addSubtitle("Chaque race apporte une petite identité de départ.");
    screen.addLine("Elle marque ton origine, tes affinités et le regard que le monde pose sur toi.");

    const std::vector<CharacterRace> races = RaceCatalog::getPlayableRaces();

    for (std::size_t i = 0; i < races.size(); ++i)
    {
        CharacterRace race = races[i];
        RaceStartingBonus bonus = RaceCatalog::getStartingBonus(race);

        std::ostringstream hint;
        hint << RaceCatalog::getGameplayIdentity(race)
             << " | PV " << bonus.maxHpBonus
             << " | Dégâts " << bonus.minDamageBonus << "/" << bonus.maxDamageBonus
             << " | Critique " << bonus.criticalDamageBonus;

        if (race == CharacterRace::Demon)
        {
            hint << " | Commerce tendu";
        }

        screen.addOption(
            static_cast<int>(i + 1),
            characterRaceToText(race),
            hint.str(),
            true,
            "character.race.select"
        );
    }

    int choice = TerminalInterface::askMenuChoiceFromOptions(
        screen,
        "Veuillez entrer un chiffre correspondant à une race affichée."
    );

    selectedRace = RaceCatalog::getPlayableRaceByChoice(choice);

    Console::clear();

    MenuScreen confirmation("RACE VALIDÉE", "character.creation.race.confirmation");
    confirmation.addLine("Race sélectionnée : " + characterRaceToText(selectedRace) + ".");
    confirmation.addLine(RaceCatalog::getShortDescription(selectedRace));

    if (selectedRace == CharacterRace::Demon)
    {
        confirmation.addLine("Note commerce : certains marchands risquent de serrer les dents en te voyant arriver.");
        confirmation.addLine("Les prix pourront être plus élevés que la norme, surtout dans les villes peu habituées aux démons.");
    }

    TerminalInterface::renderMenuScreen(confirmation, false);
    Console::waitForEnter();
    Console::clear();
}

// EN: choosePlayerClass declares or implements a focused behavior used by this module.
// FR: choosePlayerClass déclare ou implémente un comportement précis utilisé par ce module.
void Game::choosePlayerClass()
{
    MenuScreen categoryScreen("FAMILLE DE CLASSE", "character.creation.class.category");
    categoryScreen.addSubtitle("L'arène range maintenant les classes par style.");

    const std::vector<ClassCategory> categories = ClassCatalog::getClassCategories();

    for (std::size_t i = 0; i < categories.size(); ++i)
    {
        ClassCategory category = categories[i];
        categoryScreen.addOption(
            static_cast<int>(i + 1),
            classCategoryToText(category),
            std::to_string(ClassCatalog::getPlayableClassCountByCategory(category)) + " classes disponibles",
            true,
            "character.class.category.select"
        );
    }

    int categoryChoice = TerminalInterface::askMenuChoiceFromOptions(
        categoryScreen,
        "Veuillez entrer un chiffre correspondant à une famille affichée."
    );

    Console::clear();

    const std::vector<ClassOptionInfo> classOptions = ClassCatalog::getClassOptionsByCategoryChoice(categoryChoice);

    MenuScreen classScreen("CLASSE", "character.creation.class.select");
    classScreen.addSubtitle("Famille sélectionnée : " + ClassCatalog::getClassCategoryNameByChoice(categoryChoice) + ".");

    for (std::size_t i = 0; i < classOptions.size(); ++i)
    {
        const ClassOptionInfo& info = classOptions[i];
        std::ostringstream hint;
        hint << info.role
             << " | PV " << info.maxHp
             << " | Dégâts " << info.minDamage << "-" << info.maxDamage
             << " | Critique " << info.criticalDamage
             << " | Potions " << info.healingPotionCount << "/" << info.damagePotionCount;

        classScreen.addOption(
            static_cast<int>(i + 1),
            info.name,
            hint.str(),
            true,
            "character.class.select"
        );
    }

    int classChoice = TerminalInterface::askMenuChoiceFromOptions(
        classScreen,
        "Veuillez entrer un chiffre correspondant à une classe affichée."
    );

    PlayerClass chosenClass = ClassCatalog::createClassByCategoryChoice(
        categoryChoice,
        classChoice
    );

    mainPlayer = Player(playerName, chosenClass);
    mainPlayer.setRace(selectedRace);

    bool nativeBonusApplied = SpecialCharacterNativeBonus::applyIfNativeMatch(mainPlayer);

    mainPlayer.initializeStarterInventory(selectedDifficulty);

    Console::clear();

    MenuScreen confirmation("PERSONNAGE GRAVÉ", "character.creation.summary");
    confirmation.addLine(playerName + ", tu as choisi : " + characterRaceToText(selectedRace) + " / " + chosenClass.getName() + ".");
    confirmation.addLine("Famille : " + ClassCatalog::getClassCategoryNameByChoice(categoryChoice) + ".");
    confirmation.addLine("Difficulté : " + getDifficultyName() + ".");
    confirmation.addLine("Tes statistiques ont été gravées dans l'arène avec succès.");
    confirmation.addLine("Ton équipement et tes ressources de départ ont été adaptés à la difficulté.");
    confirmation.addLine("Créé le " + mainPlayer.getCreatedAtText() + " V" + mainPlayer.getCreatedForVersion());
    confirmation.addLine("Dernière adaptation faite pour la V" + mainPlayer.getLastAdaptedVersion());

    if (nativeBonusApplied)
    {
        confirmation.addLine("Bonus natif : actif.");
    }

    TerminalInterface::renderMenuScreen(confirmation, false);
    mainPlayer.displayStats();
    mainPlayer.displaySimpleEquipment();

    saveCurrentProgress("Création du personnage");

    Console::waitForEnter();
    Console::clear();
}



bool Game::isMultiplayerSession() const
{
    return partyPlayers.size() > 1;
}

std::vector<Player*> Game::getActivePartyPointers()
{
    std::vector<Player*> party;
    party.push_back(&mainPlayer);
    for (Player& player : partyPlayers)
    {
        party.push_back(&player);
    }
    return party;
}

void Game::savePartyProgress(const std::string& reason) const
{
    if (mainPlayer.isDead() && DifficultyRules::isPermanentDeath(selectedDifficulty))
    {
        SaveManager::savePlayerSnapshot(mainPlayer, accountName, selectedDifficulty);
        if (SaveManager::movePlayableCharacterToDead(accountName, mainPlayer.getName()))
        {
            MessageScreen::show(
                "REGISTRE LÉTHAL",
                "save.party.lethal.main_removed",
                {"Le registre Léthal retire " + mainPlayer.getName() + " des personnages jouables de " + accountName + "."},
                false
            );
        }
        return;
    }

    saveCurrentProgress(reason);

    for (std::size_t i = 0; i < partyPlayers.size(); ++i)
    {
        if (i >= partyAccountNames.size() || i >= partyDifficulties.size())
        {
            continue;
        }

        const Player& partyPlayer = partyPlayers[i];
        const std::string& ownerAccount = partyAccountNames[i];
        DifficultyMode playerDifficulty = partyDifficulties[i];

        if (partyPlayer.isDead() && DifficultyRules::isPermanentDeath(playerDifficulty))
        {
            SaveManager::savePlayerSnapshot(partyPlayer, ownerAccount, playerDifficulty);
            if (SaveManager::movePlayableCharacterToDead(ownerAccount, partyPlayer.getName()))
            {
                MessageScreen::show(
                    "REGISTRE LÉTHAL",
                    "save.party.lethal.member_removed",
                    {"Le registre Léthal retire " + partyPlayer.getName() + " des personnages jouables de " + ownerAccount + "."},
                    false
                );
            }
            else
            {
                MessageScreen::show(
                    "REGISTRE DES MORTS",
                    "save.party.lethal.member_refused",
                    {"Le registre des morts refuse d'emporter " + partyPlayer.getName() + " dans le registre des morts."},
                    false
                );
            }
            continue;
        }

        SaveManager::savePlayerSnapshot(partyPlayer, ownerAccount, playerDifficulty);
    }
}

bool Game::addSecondaryPlayerToParty(int playerNumber)
{
    std::vector<AccountSaveSummary> accounts = SaveManager::listAccounts();
    std::vector<AccountSaveSummary> availableAccounts;

    for (const AccountSaveSummary& account : accounts)
    {
        if (account.accountName == accountName)
        {
            continue;
        }

        bool alreadyUsed = false;
        for (const std::string& usedAccount : partyAccountNames)
        {
            if (usedAccount == account.accountName)
            {
                alreadyUsed = true;
                break;
            }
        }

        if (!alreadyUsed)
        {
            availableAccounts.push_back(account);
        }
    }

    if (availableAccounts.empty())
    {
        MenuScreen emptyScreen("JOUEUR " + std::to_string(playerNumber), "session.party.secondary.no_account");
        emptyScreen.addLine("Aucun autre compte local disponible pour le joueur " + std::to_string(playerNumber) + ".");
        emptyScreen.addLine("La coop nécessite des comptes différents, et donc des personnages différents.");
        TerminalInterface::renderMenuScreen(emptyScreen, false);
        Console::waitForEnter();
        Console::clear();
        return false;
    }

    constexpr std::size_t accountsPerPage = 10;
    std::size_t accountPage = 0;
    std::string secondaryAccount;

    while (secondaryAccount.empty())
    {
        const std::size_t totalItems = availableAccounts.size();
        const std::size_t totalPages = PagedMenu::pageCount(totalItems, accountsPerPage);
        const std::size_t first = PagedMenu::firstIndex(accountPage, accountsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(totalItems, accountPage, accountsPerPage);

        MenuScreen accountScreen("JOUEUR " + std::to_string(playerNumber), "session.party.secondary.account");
        accountScreen.addSubtitle("Compte du joueur " + std::to_string(playerNumber));
        accountScreen.addLine("Affichage : " + PagedMenu::rangeText(first, last, totalItems));

        for (std::size_t i = first; i < last; ++i)
        {
            accountScreen.addOption(
                static_cast<int>(i - first + 1),
                availableAccounts[i].accountName,
                "Compte local disponible pour cette session.",
                true,
                "session.party.account.select"
            );
        }

        PagedMenu::addNavigationOptions(accountScreen, accountPage, totalPages);

        int accountChoice = TerminalInterface::askMenuChoiceFromOptions(
            accountScreen,
            "Choisis un compte affiché."
        );
        Console::clear();

        if (accountChoice == 0)
        {
            return false;
        }

        if (accountChoice == 98 && accountPage > 0)
        {
            --accountPage;
            continue;
        }

        if (accountChoice == 99 && accountPage + 1 < totalPages)
        {
            ++accountPage;
            continue;
        }

        const int visibleCount = static_cast<int>(last - first);
        if (accountChoice >= 1 && accountChoice <= visibleCount)
        {
            secondaryAccount = availableAccounts[first + static_cast<std::size_t>(accountChoice - 1)].accountName;
        }
    }

    std::vector<CharacterSaveSummary> characters = SaveManager::listPlayableCharacters(secondaryAccount);

    if (characters.empty())
    {
        MenuScreen emptyCharacterScreen("PERSONNAGE JOUEUR " + std::to_string(playerNumber), "session.party.secondary.no_character");
        emptyCharacterScreen.addLine("Ce compte n'a aucun personnage jouable.");
        emptyCharacterScreen.addLine("Compte choisi : " + secondaryAccount + ".");
        TerminalInterface::renderMenuScreen(emptyCharacterScreen, false);
        Console::waitForEnter();
        Console::clear();
        return false;
    }

    constexpr std::size_t charactersPerPage = 8;
    std::size_t characterPage = 0;
    CharacterSaveSummary summary;
    bool characterSelected = false;

    while (!characterSelected)
    {
        const std::size_t totalItems = characters.size();
        const std::size_t totalPages = PagedMenu::pageCount(totalItems, charactersPerPage);
        const std::size_t first = PagedMenu::firstIndex(characterPage, charactersPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(totalItems, characterPage, charactersPerPage);

        MenuScreen characterScreen("PERSONNAGE JOUEUR " + std::to_string(playerNumber), "session.party.secondary.character");
        characterScreen.addSubtitle("Compte : " + secondaryAccount);
        characterScreen.addLine("Affichage : " + PagedMenu::rangeText(first, last, totalItems));

        for (std::size_t i = first; i < last; ++i)
        {
            const CharacterSaveSummary& character = characters[i];
            std::string label = character.characterName
                + " | " + character.raceName
                + " / " + character.className
                + " | Niveau " + std::to_string(character.level);

            characterScreen.addOption(
                static_cast<int>(i - first + 1),
                label,
                "Maître : " + character.currentOwnerAccountName,
                true,
                "session.party.character.select"
            );
        }

        PagedMenu::addNavigationOptions(characterScreen, characterPage, totalPages);

        int characterChoice = TerminalInterface::askMenuChoiceFromOptions(
            characterScreen,
            "Choisis un personnage affiché."
        );
        Console::clear();

        if (characterChoice == 0)
        {
            return false;
        }

        if (characterChoice == 98 && characterPage > 0)
        {
            --characterPage;
            continue;
        }

        if (characterChoice == 99 && characterPage + 1 < totalPages)
        {
            ++characterPage;
            continue;
        }

        const int visibleCount = static_cast<int>(last - first);
        if (characterChoice >= 1 && characterChoice <= visibleCount)
        {
            summary = characters[first + static_cast<std::size_t>(characterChoice - 1)];
            characterSelected = true;
        }
    }

    if (summary.currentOwnerAccountName != secondaryAccount || summary.accountName != secondaryAccount)
    {
        MenuScreen refusedScreen("MAÎTRISE REFUSÉE", "session.party.secondary.owner_refused");
        refusedScreen.addLine("Le fil de maîtrise refuse ce chargement.");
        refusedScreen.addLine("Un personnage n'a qu'un seul maître.");
        refusedScreen.addLine("Maître inscrit : " + summary.currentOwnerAccountName);
        refusedScreen.addLine("Compte choisi : " + secondaryAccount);
        TerminalInterface::renderMenuScreen(refusedScreen, false);
        Console::waitForEnter();
        Console::clear();
        return false;
    }

    Player secondaryPlayer;
    DifficultyMode secondaryDifficulty = DifficultyMode::Normal;

    if (!SaveManager::loadPlayerSnapshot(summary, secondaryPlayer, secondaryDifficulty))
    {
        MenuScreen errorScreen("CHARGEMENT IMPOSSIBLE", "session.party.secondary.load_failed");
        errorScreen.addLine("Impossible de charger ce personnage.");
        TerminalInterface::renderMenuScreen(errorScreen, false);
        Console::waitForEnter();
        Console::clear();
        return false;
    }

    partyAccountNames.push_back(secondaryAccount);
    partyDifficulties.push_back(secondaryDifficulty);
    partyPlayers.push_back(secondaryPlayer);

    MenuScreen successScreen("JOUEUR AJOUTÉ", "session.party.secondary.added");
    successScreen.addLine("Joueur " + std::to_string(playerNumber) + " ajouté : " + secondaryPlayer.getName() + " (" + secondaryAccount + ").");
    TerminalInterface::renderMenuScreen(successScreen, false);
    Console::waitForEnter();
    Console::clear();
    return true;
}

void Game::configurePartyMode()
{
    partyPlayers.clear();
    partyAccountNames.clear();
    partyDifficulties.clear();

    MenuScreen screen("SESSION", "session.party.mode");
    screen.addSubtitle("Le joueur 1 reste le point d'ancrage de la partie.");
    screen.addOption(1, "Solo", "Un seul personnage actif.", true, "session.solo");
    screen.addOption(2, "Multi local - 2 joueurs", "Un allié joueur intervient surtout en combat et récompenses individuelles.", true, "session.coop.2");
    screen.addOption(3, "Multi local - 3 joueurs", "Deux alliés joueurs avec inventaires et récompenses séparés.", true, "session.coop.3");

    int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une session affichée.");
    Console::clear();

    if (choice == 1)
    {
        MenuScreen confirmation("SESSION SOLO", "session.party.confirmation.solo");
        confirmation.addLine("Session solo sélectionnée.");
        TerminalInterface::renderMenuScreen(confirmation, false);
        Console::waitForEnter();
        Console::clear();
        return;
    }

    MenuScreen coopIntro("SESSION COOP", "session.party.confirmation.coop");
    coopIntro.addLine("Le joueur 1 reste le point d'ancrage : voyage, boss, niveau de session, événements et monstres.");
    coopIntro.addLine("Les autres joueurs interviennent surtout en combat, avec leur inventaire et leurs récompenses individuelles.");
    TerminalInterface::renderMenuScreen(coopIntro, false);
    Console::waitForEnter();
    Console::clear();

    for (int playerNumber = 2; playerNumber <= choice; ++playerNumber)
    {
        if (!addSecondaryPlayerToParty(playerNumber))
        {
            MessageScreen::show(
                "SESSION COOP",
                "session.party.partial",
                {"La session repasse sur les joueurs déjà validés."}
            );
            break;
        }
    }

    MenuScreen result("GROUPE", "session.party.result");

    if (partyPlayers.empty())
    {
        result.addLine("Aucun joueur secondaire validé. Session solo conservée.");
    }
    else
    {
        result.addLine("Groupe actif : " + std::to_string(partyPlayers.size() + 1) + " joueurs.");
    }

    TerminalInterface::renderMenuScreen(result, false);
    Console::waitForEnter();
    Console::clear();
}

// EN: chooseGameMode declares or implements a focused behavior used by this module.
// FR: chooseGameMode déclare ou implémente un comportement précis utilisé par ce module.
void Game::chooseGameMode()
{
    while (true)
    {
        MenuScreen screen("ACTIVITÉS", "activity.main");
        screen.addSubtitle("Choisis la prochaine route de " + mainPlayer.getName() + ".");
        screen.addOption(0, "Sauvegarder et quitter", "Écrire la progression puis fermer Dinotofu.", true, "activity.save_quit");
        screen.addOption(1, "Histoire", "La grande route du monde, encore scellée par les archives.", true, "activity.story");
        screen.addOption(2, "Combats", "PvE monstres, boss, groupes d'adversaires et JcJ.", true, "activity.combat");
        screen.addOption(3, "Exploration", "Biomes, plantes, matériaux, coffres, pièges, mimics et rencontres imprévues.", true, "activity.exploration");
        screen.addOption(4, "Quêtes", "Guilde, journal, demandes de PNJ et progression de quêtes.", true, "activity.quests");
        screen.addOption(5, "Boutiques / lieux visitables", "Forge, herboristerie, bibliothèque, vendeurs et lieux sociaux.", true, "activity.locations");
        screen.addOption(6, "PNJ notables", "Parler aux clients et personnages disponibles sans passer par une boutique.", true, "activity.npcs");
        screen.addOption(7, "Échange / don", "Transférer des ressources entre personnages compatibles.", true, "activity.exchange");
        screen.addOption(8, "Menu après-combat / gestion", "Ouvre le récap, l'équipement, les potions, les statistiques et les raccourcis de gestion.", true, "activity.post_combat");
        screen.addOption(9, "Information sur toutes les options", "Ouvre un guide clair sur les routes possibles.", true, "activity.info");

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Veuillez choisir une activité affichée."
        );

        Console::clear();

        if (choice == 0)
        {
            saveCurrentProgress("Sauvegarder et quitter");
            MessageScreen::show(
                "SAUVEGARDE",
                "activity.save_quit.done",
                {"Progression sauvegardée. Fermeture de Dinotofu."},
                false
            );
            std::exit(0);
        }

        if (choice == 8)
        {
            const bool keepPlaying = openPostCombatMenu();
            if (!keepPlaying)
            {
                std::exit(0);
            }
            continue;
        }

        if (choice == 9)
        {
            displayActivityInformation();
            continue;
        }

        if (choice == 2)
        {
            MenuScreen combatScreen("COMBATS", "activity.combat.menu");
            combatScreen.addBackOption();
            combatScreen.addOption(1, "PvP IA", "Duel contre une IA, avec personnages spéciaux possibles selon le mode.", true, "combat.ai_pvp");
            combatScreen.addOption(2, "PvP 2 joueurs / JcJ", "Duel local amical ou mortel selon les comptes, clones, altérations et difficultés.", true, "combat.local_pvp");
            combatScreen.addOption(3, "PvE monstres", "Vagues de monstres, loots, matériaux, qualité de récupération et progression.", true, "combat.monster_pve");
            combatScreen.addOption(4, "PvE Boss", "Combat contre un boss avec identité, décryptage et fragments spéciaux.", true, "combat.boss_pve");

            int combatChoice = TerminalInterface::askMenuChoiceFromOptions(
                combatScreen,
                "Veuillez choisir un combat affiché."
            );
            Console::clear();

            if (combatChoice == 0)
            {
                continue;
            }

            switch (combatChoice)
            {
                case 1:
                    selectedMode = GameMode::AIPvp;
                    return;
                case 2:
                    selectedMode = GameMode::TwoPlayerPvp;
                    return;
                case 3:
                    selectedMode = GameMode::MonsterPve;
                    return;
                case 4:
                    selectedMode = GameMode::BossPve;
                    return;
                default:
                    break;
            }
        }
        else if (choice == 1)
        {
            selectedMode = GameMode::Story;
            return;
        }
        else if (choice == 3)
        {
            selectedMode = GameMode::Exploration;
            return;
        }
        else if (choice == 4)
        {
            selectedMode = GameMode::Challenges;
            return;
        }
        else if (choice == 5)
        {
            selectedMode = GameMode::Locations;
            return;
        }
        else if (choice == 6)
        {
            selectedMode = GameMode::NotableNpcs;
            return;
        }
        else if (choice == 7)
        {
            selectedMode = GameMode::Exchange;
            return;
        }
    }
}

// EN: displaySelectedMode declares or implements a focused behavior used by this module.
// FR: displaySelectedMode déclare ou implémente un comportement précis utilisé par ce module.
void Game::displaySelectedMode()
{
    Console::clear();

    MenuScreen screen("ACTIVITÉ SÉLECTIONNÉE", "activity.selected");
    screen.addLine("Activité : " + getSelectedModeName());
    screen.addLine("Difficulté : " + getDifficultyName());

    if (isMultiplayerSession())
    {
        screen.addLine("Groupe actif : " + std::to_string(partyPlayers.size() + 1) + " joueurs.");
    }

    TerminalInterface::renderMenuScreen(screen, false);
    Console::waitForEnter();
    Console::clear();
}


std::string Game::getSelectedModeName() const
{
    switch (selectedMode)
    {
        case GameMode::Story:
            return "Histoire";
        case GameMode::AIPvp:
            return "Combat - PvP IA";
        case GameMode::TwoPlayerPvp:
            return "Combat - PvP 2 joueurs / JcJ";
        case GameMode::MonsterPve:
            return "Combat - PvE monstres";
        case GameMode::BossPve:
            return "Combat - PvE Boss";
        case GameMode::Challenges:
            return "Quêtes";
        case GameMode::Exploration:
            return "Exploration";
        case GameMode::Locations:
            return "Boutiques / lieux visitables";
        case GameMode::NotableNpcs:
            return "PNJ notables";
        case GameMode::Exchange:
            return "Échange / don";
    }

    return "Activité inconnue";
}

Game::CombatRecapSnapshot Game::captureCombatRecapSnapshot() const
{
    CombatRecapSnapshot snapshot;
    snapshot.level = mainPlayer.getLevel();
    snapshot.experience = mainPlayer.getExperience();
    snapshot.hp = mainPlayer.getHp();
    snapshot.maxHp = mainPlayer.getMaxHp();
    snapshot.gold = mainPlayer.getInventory().getGold();
    snapshot.victories = mainPlayer.getVictories();
    snapshot.defeats = mainPlayer.getDefeats();
    snapshot.escapes = mainPlayer.getEscapes();
    snapshot.enemiesKilled = mainPlayer.getEnemiesKilled();
    snapshot.bossesKilled = mainPlayer.getBossesKilled();
    return snapshot;
}

void Game::updateLastCombatRecap(const CombatRecapSnapshot& beforeSnapshot)
{
    lastCombatRecap.available = true;
    lastCombatRecap.modeName = getSelectedModeName();
    lastCombatRecap.difficultyName = getDifficultyName();
    lastCombatRecap.before = beforeSnapshot;
    lastCombatRecap.after = captureCombatRecapSnapshot();
}

void Game::displayLastCombatRecap() const
{
    if (!lastCombatRecap.available)
    {
        MessageScreen::show(
            "DERNIER RÉCAP",
            "post_combat.last_recap.empty",
            {
                "Aucun combat récent enregistré dans cette session.",
                "Lance un combat pour que le registre compare l'avant et l'après."
            }
        );
        return;
    }

    const CombatRecapSnapshot& before = lastCombatRecap.before;
    const CombatRecapSnapshot& after = lastCombatRecap.after;

    MessageScreen::show(
        "DERNIER RÉCAP DE COMBAT",
        "post_combat.last_recap.detail",
        {
            "Activité : " + lastCombatRecap.modeName,
            "Difficulté : " + lastCombatRecap.difficultyName,
            "",
            "Avant : niveau " + std::to_string(before.level)
                + " | XP " + std::to_string(before.experience)
                + " | PV " + std::to_string(before.hp) + "/" + std::to_string(before.maxHp)
                + " | Or " + std::to_string(before.gold),
            "Après : niveau " + std::to_string(after.level)
                + " | XP " + std::to_string(after.experience)
                + " | PV " + std::to_string(after.hp) + "/" + std::to_string(after.maxHp)
                + " | Or " + std::to_string(after.gold),
            "",
            "Variations :",
            "- Niveau : " + std::to_string(after.level - before.level),
            "- Expérience : " + std::to_string(after.experience - before.experience),
            "- PV actuels : " + std::to_string(after.hp - before.hp),
            "- PV max : " + std::to_string(after.maxHp - before.maxHp),
            "- Or : " + std::to_string(after.gold - before.gold),
            "- Victoires : " + std::to_string(after.victories - before.victories),
            "- Défaites : " + std::to_string(after.defeats - before.defeats),
            "- Fuites : " + std::to_string(after.escapes - before.escapes),
            "- Ennemis vaincus : " + std::to_string(after.enemiesKilled - before.enemiesKilled),
            "- Boss vaincus : " + std::to_string(after.bossesKilled - before.bossesKilled)
        }
    );
}

// EN: displayActivityInformation declares or implements a focused behavior used by this module.
// FR: displayActivityInformation déclare ou implémente un comportement précis utilisé par ce module.
void Game::displayActivityInformation() const
{
    MenuScreen screen("INFORMATION SUR LES ACTIVITÉS", "activity.info");
    screen.addLine("Histoire : grande route principale, encore scellée par les archives du monde.");
    screen.addLine("Combats : affrontements volontaires contre IA, joueurs, monstres ou boss.");
    screen.addLine("Exploration : sortie de terrain par biome avec plantes, matériaux, trésors, pièges, mimics et combats inattendus.");
    screen.addLine("Quêtes : journal, guilde, demandes de PNJ, quêtes terminées et validation des objectifs.");
    screen.addLine("Boutiques / lieux visitables : forge, herboristerie, bibliothèque, vendeurs et lieux sociaux.");
    screen.addLine("PNJ notables : accès direct aux clients ou personnages disponibles.");
    screen.addLine("Échange / don : transfert protégé d'objets ou d'or entre personnages compatibles.");
    screen.addLine("Menu après-combat / gestion : relire le dernier état, gérer équipement, potions, progression et statistiques sans lancer de combat.");
    screen.addFooterLine("Certaines portes restent fermées. Le monde te laisse progresser par ses guildes, ses routes et ses combats.");

    TerminalInterface::renderMenuScreen(screen, false);
    Console::waitForEnter();
    Console::clear();
}

// EN: launchSelectedMode declares or implements a focused behavior used by this module.
// FR: launchSelectedMode déclare ou implémente un comportement précis utilisé par ce module.
void Game::launchSelectedMode()
{
    Combat combat;

    if (selectedMode == GameMode::Story)
    {
        launchStoryModePlaceholder();
        saveCurrentProgress("Passage dans le mode histoire");
    }
    else if (selectedMode == GameMode::Challenges)
    {
        launchChallengeBoard();
        saveCurrentProgress("Quêtes");
    }
    else if (selectedMode == GameMode::Exploration)
    {
        QuestMenu::openExploration(mainPlayer, selectedDifficulty);
        saveCurrentProgress("Exploration");
    }
    else if (selectedMode == GameMode::Locations)
    {
        QuestMenu::openLocations(mainPlayer);
        saveCurrentProgress("Lieux visitables");
    }
    else if (selectedMode == GameMode::NotableNpcs)
    {
        QuestMenu::openNotableNpcMenu(mainPlayer);
        saveCurrentProgress("PNJ notables");
    }
    else if (selectedMode == GameMode::Exchange)
    {
        openExchangeMenu();
        saveCurrentProgress("Échange entre personnages");
    }
    else
    {
        const CombatRecapSnapshot beforeCombatSnapshot = captureCombatRecapSnapshot();

        mainPlayer.recordCombatStarted();
        ShopTransactionSystem::clearBuybackAfterCombat();

        switch (selectedMode)
        {
            case GameMode::AIPvp:
            {
                combat.launchAIPvp(mainPlayer);
                break;
            }

            case GameMode::TwoPlayerPvp:
            {
                combat.launchTwoPlayerPvp(mainPlayer, accountName, selectedDifficulty);
                break;
            }

            case GameMode::MonsterPve:
            {
                if (isMultiplayerSession())
                {
                    std::vector<Player*> party = getActivePartyPointers();
                    combat.launchMonsterPveTeam(party, selectedDifficulty);
                }
                else
                {
                    combat.launchMonsterPve(mainPlayer, selectedDifficulty);
                }
                break;
            }

            case GameMode::BossPve:
            {
                if (isMultiplayerSession())
                {
                    std::vector<Player*> party = getActivePartyPointers();
                    combat.launchBossPveTeam(party, selectedDifficulty);
                }
                else
                {
                    combat.launchBossPve(mainPlayer, selectedDifficulty);
                }
                break;
            }

            case GameMode::Story:
            case GameMode::Challenges:
            case GameMode::Exploration:
            case GameMode::Locations:
            case GameMode::NotableNpcs:
            case GameMode::Exchange:
                break;
        }

        updateLastCombatRecap(beforeCombatSnapshot);
        ShopRotationSystem::markShopsDirtyAfterCombat();

        if (mainPlayer.isDead() && DifficultyRules::isPermanentDeath(selectedDifficulty))
        {
            saveCurrentProgress("Mort définitive");

            if (SaveManager::movePlayableCharacterToDead(accountName, mainPlayer.getName()))
            {
                MessageScreen::show(
                    "REGISTRE DES MORTS",
                    "combat.lethal.main_moved",
                    {
                        "Le personnage a été déplacé dans le registre des morts.",
                        "Il ne sera plus disponible dans les personnages jouables."
                    },
                    false
                );
            }
            else
            {
                MessageScreen::show(
                    "REGISTRE DES MORTS",
                    "combat.lethal.main_move_failed",
                    {
                        "Le registre des morts refuse de se fermer correctement autour de ce personnage.",
                        "La sauvegarde de mort a tout de même été tentée."
                    },
                    false
                );
            }

            DeathPenaltySystem::displayLethalDeathCorruption();
            Console::waitForEnter();
            return;
        }

        savePartyProgress("Fin de combat");
        QuestMenu::maybeOfferRandomInterception(mainPlayer, selectedDifficulty);
        savePartyProgress("Événement de quête éventuel");
    }

    bool continuePlaying = openPostCombatMenu();

    if (continuePlaying)
    {
        chooseGameMode();
        displaySelectedMode();
        launchSelectedMode();
        return;
    }

    savePartyProgress("Fin de session");
}

// EN: launchStoryModePlaceholder declares or implements a focused behavior used by this module.
// FR: launchStoryModePlaceholder déclare ou implémente un comportement précis utilisé par ce module.
void Game::launchStoryModePlaceholder()
{
    MessageScreen::show(
        "HISTOIRE",
        "story.placeholder.sealed_road",
        {
            "La grande route de l'Histoire se dresse devant toi.",
            "Les portes principales restent scellées par des archives anciennes.",
            "Ce que murmurent les archives :",
            "Tu n'es pas seulement un combattant d'arène. Tu es un personnage inscrit dans un monde",
            "où les quêtes, les guildes, les boss, les matériaux rares et les choix laissent déjà des traces.",
            "Les guildes, l'exploration, les monstres et les boss restent les routes ouvertes pour faire grandir ton personnage.",
            "Quand les sceaux céderont, cette route reprendra là où ton personnage aura grandi."
        }
    );
}

// EN: launchChallengeBoard declares or implements a focused behavior used by this module.
// FR: launchChallengeBoard déclare ou implémente un comportement précis utilisé par ce module.
void Game::launchChallengeBoard()
{
    QuestMenu::openQuestHub(mainPlayer);
}

// EN: openPostCombatMenu declares or implements a focused behavior used by this module.
// FR: openPostCombatMenu déclare ou implémente un comportement précis utilisé par ce module.
bool Game::openPostCombatMenu()
{
    bool menuOpen = true;

    while (menuOpen)
    {
        const bool hasLastCombatRecap = lastCombatRecap.available;
        int maxChoice = PostCombatMenu::getMaxChoice(mainPlayer, hasLastCombatRecap);

        PostCombatMenu::display(mainPlayer, hasLastCombatRecap);

        std::string input;
        Console::readLine(input, true);

        std::istringstream stream(input);
        int choice = -1;
        char extraCharacter;

        bool isCleanNumber = false;

        if (stream >> choice)
        {
            isCleanNumber = !(stream >> extraCharacter);
        }

        if (!isCleanNumber)
        {
            Console::clear();

            if (CheatManager::tryActivateHiddenCode(mainPlayer, selectedDifficulty, input))
            {
                saveCurrentProgress("Altération cachée");
                Console::waitForEnter();
                Console::clear();
            }
            else
            {
                MessageScreen::show(
                    "ENTRÉE INVALIDE",
                    "post_combat.invalid_input",
                    {"Entrée invalide."}
                );
            }

            continue;
        }

        Console::clear();

        if (choice < 0 || choice > maxChoice)
        {
            MessageScreen::show(
                "CHOIX INVALIDE",
                "post_combat.choice_out_of_range",
                {"Veuillez choisir une option affichée."}
            );
            continue;
        }

        if (choice == 0)
        {
            return true;
        }
        else if (choice == 1)
        {
            ShopMenu::open(mainPlayer);
            saveCurrentProgress("Passage en boutique");
        }
        else if (choice == 2)
        {
            StatisticsMenu::open(mainPlayer, selectedDifficulty);
        }
        else if (choice == 3)
        {
            InventoryMenu::open(mainPlayer);
            saveCurrentProgress("Inventaire après-combat");
            Console::clear();
        }
        else if (choice == 4)
        {
            AttributeMenu::displayLockedDevelopmentMessage();
            Console::waitForEnter();
            Console::clear();
        }
        else if (choice == 5)
        {
            saveCurrentProgress("Sauvegarde rapide");
            Console::waitForEnter();
            Console::clear();
        }
        else if (choice == 6)
        {
            saveCurrentProgress("Sauvegarder et quitter");
            MessageScreen::show(
                "SAUVEGARDE",
                "post_combat.save_quit.done",
                {"Progression sauvegardée. Fermeture de Dinotofu."}
            );
            return false;
        }
        else if (choice == 7)
        {
            QuestMenu::consultOnly(mainPlayer);
        }
        else if (choice == 8)
        {
            QuestMenu::openLocations(mainPlayer);
            saveCurrentProgress("Lieux visitables");
        }
        else if (choice == 9)
        {
            QuestMenu::openNotableNpcMenu(mainPlayer);
            saveCurrentProgress("PNJ notables");
        }
        else if (choice == 10)
        {
            openExchangeMenu();
            saveCurrentProgress("Échange entre personnages");
        }
        else if (choice == 11)
        {
            mainPlayer.displaySkillProgress();
            Console::waitForEnter();
            Console::clear();
        }
        else if (choice == 12)
        {
            mainPlayer.displaySimpleEquipment();
            Console::waitForEnter();
            Console::clear();
        }
        else if (choice == 13)
        {
            displayLastCombatRecap();
        }
        else if (choice == 14 && mainPlayer.isAlteredByCheats())
        {
            CheatManager::openAlteredDataMenu(mainPlayer, selectedDifficulty);
            saveCurrentProgress("Données altérées");
        }
    }

    return false;
}


// EN: openExchangeMenu declares or implements a focused behavior used by this module.
// FR: openExchangeMenu déclare ou implémente un comportement précis utilisé par ce module.
void Game::openExchangeMenu()
{
    std::vector<AccountSaveSummary> accounts = SaveManager::listAccounts();

    if (accounts.empty())
    {
        MenuScreen emptyScreen("ÉCHANGE / DON", "exchange.no_account");
        emptyScreen.addLine("Aucun autre compte disponible pour un échange.");
        TerminalInterface::renderMenuScreen(emptyScreen, false);
        Console::waitForEnter();
        Console::clear();
        return;
    }

    MenuScreen accountScreen("ÉCHANGE / DON", "exchange.account.select");
    accountScreen.addSubtitle("Choisis le compte cible.");
    accountScreen.addBackOption();

    for (int i = 0; i < static_cast<int>(accounts.size()); ++i)
    {
        accountScreen.addOption(
            i + 1,
            accounts[i].accountName,
            accounts[i].accountName == accountName ? "Ton compte actuel : seuls les autres personnages peuvent être ciblés." : "Compte local disponible.",
            true,
            "exchange.account.select"
        );
    }

    int accountChoice = TerminalInterface::askMenuChoiceFromOptions(
        accountScreen,
        "Veuillez choisir un compte affiché."
    );
    Console::clear();

    if (accountChoice == 0)
    {
        return;
    }

    std::string targetAccount = accounts[accountChoice - 1].accountName;
    std::vector<CharacterSaveSummary> characters = SaveManager::listPlayableCharacters(targetAccount);

    if (characters.empty())
    {
        MenuScreen emptyCharacterScreen("PERSONNAGE CIBLE", "exchange.character.empty");
        emptyCharacterScreen.addLine("Ce compte n'a aucun personnage jouable.");
        emptyCharacterScreen.addLine("Compte : " + targetAccount);
        TerminalInterface::renderMenuScreen(emptyCharacterScreen, false);
        Console::waitForEnter();
        Console::clear();
        return;
    }

    MenuScreen characterScreen("PERSONNAGE CIBLE", "exchange.character.select");
    characterScreen.addSubtitle("Compte cible : " + targetAccount);
    characterScreen.addBackOption();

    for (int i = 0; i < static_cast<int>(characters.size()); ++i)
    {
        const CharacterSaveSummary& character = characters[i];
        std::string label = character.characterName
            + " | " + character.raceName
            + " / " + character.className
            + " | Niveau " + std::to_string(character.level);

        bool sameCharacter = targetAccount == accountName && character.characterName == mainPlayer.getName();

        characterScreen.addOption(
            i + 1,
            label,
            sameCharacter ? "C'est ton personnage actuel : échange impossible avec soi-même." : "Maître : " + character.currentOwnerAccountName,
            true,
            "exchange.character.select"
        );
    }

    int characterChoice = TerminalInterface::askMenuChoiceFromOptions(
        characterScreen,
        "Veuillez choisir un personnage affiché."
    );
    Console::clear();

    if (characterChoice == 0)
    {
        return;
    }

    CharacterSaveSummary targetSummary = characters[characterChoice - 1];

    if (targetAccount == accountName && targetSummary.characterName == mainPlayer.getName())
    {
        Console::clear();
        MessageScreen::show(
            "ÉCHANGE IMPOSSIBLE",
            "exchange.forbidden.same_character",
            {"Tu ne peux pas échanger avec le même personnage."}
        );
        return;
    }

    Player targetPlayer;
    DifficultyMode targetDifficulty = DifficultyMode::Normal;

    if (!SaveManager::loadPlayerSnapshot(targetSummary, targetPlayer, targetDifficulty))
    {
        Console::clear();
        MessageScreen::show(
            "ÉCHANGE IMPOSSIBLE",
            "exchange.load_target_failed",
            {"Impossible de charger le personnage cible."}
        );
        return;
    }

    if (mainPlayer.isAlteredByCheats() || targetPlayer.isAlteredByCheats())
    {
        Console::clear();
        MessageScreen::show(
            "ÉCHANGE IMPOSSIBLE",
            "exchange.forbidden.altered",
            {
                "Échange impossible.",
                "Un personnage altéré ne peut pas transférer de ressources réelles."
            }
        );
        return;
    }

    if (mainPlayer.isClone() || targetPlayer.isClone())
    {
        Console::clear();
        MessageScreen::show(
            "ÉCHANGE IMPOSSIBLE",
            "exchange.forbidden.clone",
            {
                "Un clone ne peut pas donner ou recevoir d'objets réels.",
                "Le registre refuse les silhouettes copiées dans les échanges réels."
            }
        );
        return;
    }

    bool currentIsLethal = selectedDifficulty == DifficultyMode::Lethal;
    bool targetIsLethal = targetDifficulty == DifficultyMode::Lethal;

    if (currentIsLethal != targetIsLethal)
    {
        Console::clear();
        MessageScreen::show(
            "ÉCHANGE IMPOSSIBLE",
            "exchange.forbidden.lethal_mismatch",
            {
                "Un personnage Léthal est considéré comme une vraie existence.",
                "Un personnage non Léthal reste une simulation plus sûre.",
                "Pour éviter les abus, il faut deux Léthal ou deux non Léthal."
            }
        );
        return;
    }

    bool open = true;

    while (open)
    {
        Console::clear();

        MenuScreen exchangeScreen("ÉCHANGE / DON", "exchange.action");
        exchangeScreen.addLine("Source principale : " + mainPlayer.getName());
        exchangeScreen.addLine("Cible : " + targetPlayer.getName() + " (" + targetAccount + ")");
        exchangeScreen.addBackOption();
        exchangeScreen.addOption(1, "Donner de l'or", "Transfert direct depuis " + mainPlayer.getName() + ".", true, "exchange.give.gold");
        exchangeScreen.addOption(2, "Donner une arme", "Impossible avec l'arme équipée.", true, "exchange.give.weapon");
        exchangeScreen.addOption(3, "Donner une armure", "Impossible avec l'armure portée.", true, "exchange.give.armor");
        exchangeScreen.addOption(4, "Donner un consommable", "Transfert d'un objet consommable.", true, "exchange.give.consumable");
        exchangeScreen.addOption(5, "Donner un matériau", "Transfert avec quantité choisie.", true, "exchange.give.material");
        exchangeScreen.addOption(6, "Recevoir depuis le personnage cible", "Inverse la source et la cible pour cette action.", true, "exchange.receive");
        exchangeScreen.addFooterLine("L'estimation de valeur s'affiche après le choix pour garder l'écran lisible.");

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            exchangeScreen,
            "Veuillez choisir une option affichée."
        );

        Player* giver = &mainPlayer;
        Player* receiver = &targetPlayer;

        if (choice == 0)
        {
            break;
        }

        if (choice == 6)
        {
            giver = &targetPlayer;
            receiver = &mainPlayer;

            Console::clear();
            MenuScreen receiveScreen("RECEVOIR", "exchange.receive.type");
            receiveScreen.addLine("Depuis : " + giver->getName());
            receiveScreen.addLine("Vers : " + receiver->getName());
            receiveScreen.addBackOption("Annuler");
            receiveScreen.addOption(1, "Or", "Transférer une quantité d'or.", true, "exchange.receive.gold");
            receiveScreen.addOption(2, "Arme", "Choisir une arme non équipée.", true, "exchange.receive.weapon");
            receiveScreen.addOption(3, "Armure", "Choisir une armure non portée.", true, "exchange.receive.armor");
            receiveScreen.addOption(4, "Consommable", "Choisir un consommable.", true, "exchange.receive.consumable");
            receiveScreen.addOption(5, "Matériau", "Choisir un matériau et une quantité.", true, "exchange.receive.material");

            choice = TerminalInterface::askMenuChoiceFromOptions(
                receiveScreen,
                "Veuillez choisir une ressource affichée."
            );

            if (choice == 0)
            {
                continue;
            }
        }

        Console::clear();
        displayExchangeValueEstimation(*giver, *receiver);

        if (choice == 1)
        {
            int amount = MessageScreen::askQuantity(
                "OR À TRANSFÉRER",
                "exchange.gold.quantity",
                {
                    giver->getName() + " possède " + std::to_string(giver->getInventory().getGold()) + " or.",
                    "Montant à transférer ?"
                },
                0,
                giver->getInventory().getGold(),
                "Montant invalide."
            );

            if (amount > 0 && giver->getInventory().spendGold(amount))
            {
                receiver->getInventory().earnGold(amount);
                MessageScreen::show("ÉCHANGE EFFECTUÉ", "exchange.gold.success", {std::to_string(amount) + " or transféré."}, false);
            }
            else
            {
                MessageScreen::show("ÉCHANGE ANNULÉ", "exchange.gold.none", {"Aucun or transféré."}, false);
            }
        }
        else if (choice == 2)
        {
            int index = askExchangeWeaponIndex(*giver);

            if (index >= 0)
            {
                Weapon weapon = giver->getInventory().getWeapon(index);
                receiver->getInventory().addWeapon(weapon);
                giver->getInventory().removeWeapon(index);
                MessageScreen::show("ÉCHANGE EFFECTUÉ", "exchange.weapon.success", {"Arme transférée : " + weapon.getName() + "."}, false);
            }
            else
            {
                MessageScreen::show("ÉCHANGE ANNULÉ", "exchange.weapon.none", {"Aucune arme transférée."}, false);
            }
        }
        else if (choice == 3)
        {
            int index = askExchangeArmorIndex(*giver);

            if (index >= 0)
            {
                Armor armor = giver->getInventory().getArmor(index);
                receiver->getInventory().addArmor(armor);
                giver->getInventory().removeArmor(index);
                MessageScreen::show("ÉCHANGE EFFECTUÉ", "exchange.armor.success", {"Armure transférée : " + armor.getName() + "."}, false);
            }
            else
            {
                MessageScreen::show("ÉCHANGE ANNULÉ", "exchange.armor.none", {"Aucune armure transférée."}, false);
            }
        }
        else if (choice == 4)
        {
            int index = askExchangeConsumableIndex(*giver);

            if (index >= 0)
            {
                Consumable consumable = giver->getInventory().getConsumable(index);
                receiver->getInventory().addConsumable(consumable);
                giver->getInventory().removeConsumable(index);
                MessageScreen::show("ÉCHANGE EFFECTUÉ", "exchange.consumable.success", {"Consommable transféré : " + consumable.getName() + "."}, false);
            }
            else
            {
                MessageScreen::show("ÉCHANGE ANNULÉ", "exchange.consumable.none", {"Aucun consommable transféré."}, false);
            }
        }
        else if (choice == 5)
        {
            int index = askExchangeMaterialIndex(*giver);

            if (index >= 0)
            {
                Material material = giver->getInventory().getMaterial(index);
                int amount = MessageScreen::askQuantity(
                    "QUANTITÉ À TRANSFÉRER",
                    "exchange.material.quantity",
                    {
                        "Matériau : " + material.getName(),
                        "Maximum transférable : x" + std::to_string(material.getQuantity())
                    },
                    1,
                    material.getQuantity(),
                    "Quantité invalide."
                );
                material.setQuantity(amount);
                receiver->getInventory().addMaterial(material);
                giver->getInventory().removeMaterialQuantity(index, amount);
                MessageScreen::show("ÉCHANGE EFFECTUÉ", "exchange.material.success", {"Matériau transféré : " + material.getName() + " x" + std::to_string(amount) + "."}, false);
            }
            else
            {
                MessageScreen::show("ÉCHANGE ANNULÉ", "exchange.material.none", {"Aucun matériau transféré."}, false);
            }
        }

        SaveManager::savePlayerSnapshot(mainPlayer, accountName, selectedDifficulty);
        SaveManager::savePlayerSnapshot(targetPlayer, targetAccount, targetDifficulty);

        MessageScreen::show(
            "ÉCHANGE SAUVEGARDÉ",
            "exchange.saved",
            {"Le transfert est enregistré dans les deux registres."}
        );
    }

    Console::clear();
}

// EN: saveCurrentProgress declares or implements a focused behavior used by this module.
// FR: saveCurrentProgress déclare ou implémente un comportement précis utilisé par ce module.
void Game::saveCurrentProgress(const std::string& reason) const
{
    if (mainPlayer.getName().empty() || mainPlayer.getName() == "Inconnu")
    {
        return;
    }

    if (SaveManager::savePlayerSnapshot(mainPlayer, accountName, selectedDifficulty))
    {
        MessageScreen::show(
            "SAUVEGARDE",
            "save.current_progress.ok",
            {
                "Sauvegarde préparée : " + reason + ".",
                "Chemin : " + SaveManager::getCharacterSavePath(accountName, mainPlayer.getName())
            },
            false
        );
    }
    else
    {
        MessageScreen::show(
            "SAUVEGARDE",
            "save.current_progress.failed",
            {"Sauvegarde impossible pour le moment."},
            false
        );
    }
}

std::string Game::getDifficultyName() const
{
    switch (selectedDifficulty)
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
