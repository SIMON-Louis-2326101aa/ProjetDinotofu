// EN: Game.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Game.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "core/Game.hpp"
#include "core/Console.hpp"
#include "core/Random.hpp"
#include "class_system/ClassCatalog.hpp"
#include "combat/Combat.hpp"
#include "character/RaceCatalog.hpp"
#include "character/SpecialCharacterNativeBonus.hpp"
#include "save/SaveManager.hpp"
#include "save/menu/AccountMenu.hpp"
#include "save/menu/CharacterMenu.hpp"
#include "economy/shop/ShopRotationSystem.hpp"
#include "interface/menu/shop/ShopMenu.hpp"
#include "interface/menu/progression/AttributeMenu.hpp"
#include "interface/menu/progression/StatisticsMenu.hpp"
#include "interface/menu/quest/QuestMenu.hpp"
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
        std::cout << "Estimation de la valeur de l'échangeur " << first.getName()
                  << " : " << estimatePlayerTradeValue(first) << " pièces." << std::endl;
        std::cout << "Estimation de la valeur de l'échangeur " << second.getName()
                  << " : " << estimatePlayerTradeValue(second) << " pièces." << std::endl;
        std::cout << std::endl;
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

    chooseGameMode();
    displaySelectedMode();
    launchSelectedMode();
}

// EN: displayIntroduction declares or implements a focused behavior used by this module.
// FR: displayIntroduction déclare ou implémente un comportement précis utilisé par ce module.
void Game::displayIntroduction()
{
    std::cout << "Bonjour voyageur, et bienvenue dans Dinotofu." << std::endl;
    Console::pauseSeconds(1);

    std::cout << "Un monde de fantaisie, d'arènes et de baston," << std::endl;
    std::cout << "où chaque choix peut transformer un simple combattant en légende." << std::endl;
    std::cout << std::endl;

    Console::waitForEnter();
    Console::clear();
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
    std::cout << "Choisis la difficulté de ton personnage." << std::endl;
    std::cout << "Ce choix influence le kit de départ, les récompenses, la mort et le respawn." << std::endl;
    std::cout << std::endl;

    std::cout << "1 : Facile" << std::endl;
    std::cout << "    Plus d'or, plus de sécurité, retour à 75% PV après une mort non définitive." << std::endl;
    std::cout << std::endl;

    std::cout << "2 : Normal" << std::endl;
    std::cout << "    L'expérience Dinotofu standard." << std::endl;
    std::cout << std::endl;

    std::cout << "3 : Difficile" << std::endl;
    std::cout << "    Moins de ressources, pénalités plus dures, retour à 30% PV." << std::endl;
    std::cout << std::endl;

    std::cout << "4 : Cauchemar" << std::endl;
    std::cout << "    Très punitif, retour à 10% PV, et la mort commence vraiment à avoir des dents." << std::endl;
    std::cout << std::endl;

    std::cout << "5 : Léthal" << std::endl;
    std::cout << "    Mort définitive prévue plus tard. Les statistiques de mort sont corrompues." << std::endl;
    std::cout << std::endl;

    std::cout << "> ";

    int choice = Console::askNumberBetween(
        1,
        5,
        "Veuillez entrer un chiffre valide entre 1 et 5."
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

    std::cout << "Difficulté sélectionnée : " << getDifficultyName() << "." << std::endl;
    std::cout << "Ton départ sera ajusté en conséquence." << std::endl;

    if (specialIdentityValidated)
    {
        std::cout << "Identité spéciale reconnue : le choix de race est verrouillé par son histoire." << std::endl;
        std::cout << "Race imposée : " << characterRaceToText(selectedRace) << "." << std::endl;
    }
    std::cout << std::endl;

    Console::waitForEnter();
    Console::clear();
}

// EN: choosePlayerRace declares or implements a focused behavior used by this module.
// FR: choosePlayerRace déclare ou implémente un comportement précis utilisé par ce module.
void Game::choosePlayerRace()
{
    std::cout << "Choisis ta race." << std::endl;
    std::cout << "Chaque race apporte une petite identité de départ." << std::endl;
    std::cout << "Plus tard, elle pourra aussi influencer les dialogues, les ventes, les résistances et certaines quêtes." << std::endl;
    std::cout << std::endl;

    RaceCatalog::displayPlayableRaces();

    std::cout << "Veuillez entrer uniquement le chiffre correspondant." << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(
        1,
        RaceCatalog::getPlayableRaceCount(),
        "Veuillez entrer un chiffre correspondant à une race affichée."
    );

    selectedRace = RaceCatalog::getPlayableRaceByChoice(choice);

    Console::clear();

    std::cout << "Race sélectionnée : " << characterRaceToText(selectedRace) << "." << std::endl;
    std::cout << RaceCatalog::getShortDescription(selectedRace) << std::endl;

    if (selectedRace == CharacterRace::Demon)
    {
        std::cout << std::endl;
        std::cout << "Note commerce : certains marchands risquent de serrer les dents en te voyant arriver." << std::endl;
        std::cout << "Les prix pourront être plus élevés que la norme, surtout dans les villes peu habituées aux démons." << std::endl;
    }

    std::cout << std::endl;

    Console::waitForEnter();
    Console::clear();
}

// EN: choosePlayerClass declares or implements a focused behavior used by this module.
// FR: choosePlayerClass déclare ou implémente un comportement précis utilisé par ce module.
void Game::choosePlayerClass()
{
    std::cout << "Choisis la famille de classe qui t'intéresse." << std::endl;
    std::cout << "Comme il commence à y avoir pas mal de choix, l'arène range maintenant les classes par style." << std::endl;
    std::cout << std::endl;

    ClassCatalog::displayClassCategories();

    std::cout << std::endl;
    std::cout << "Veuillez entrer uniquement le chiffre correspondant." << std::endl;
    std::cout << "> ";

    int categoryChoice = Console::askNumberBetween(
        1,
        ClassCatalog::getClassCategoryCount(),
        "Veuillez entrer un chiffre correspondant à une famille affichée."
    );

    Console::clear();

    std::cout << "Famille sélectionnée : "
              << ClassCatalog::getClassCategoryNameByChoice(categoryChoice)
              << "."
              << std::endl;
    std::cout << "Choisis maintenant ta classe." << std::endl;
    std::cout << std::endl;

    ClassCatalog::displayClassesByCategoryChoice(categoryChoice);

    int maxClassChoice = ClassCatalog::getPlayableClassCountByCategoryChoice(categoryChoice);

    std::cout << "Veuillez entrer uniquement le chiffre correspondant." << std::endl;
    std::cout << "> ";

    int classChoice = Console::askNumberBetween(
        1,
        maxClassChoice,
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

    std::cout << playerName << ", tu as choisi : "
              << characterRaceToText(selectedRace)
              << " / "
              << chosenClass.getName()
              << "."
              << std::endl;
    std::cout << "Famille : " << ClassCatalog::getClassCategoryNameByChoice(categoryChoice) << "." << std::endl;
    std::cout << "Difficulté : " << getDifficultyName() << "." << std::endl;
    std::cout << "Tes statistiques ont été gravées dans l'arène avec succès." << std::endl;
    std::cout << "Ton équipement et tes ressources de départ ont été adaptés à la difficulté." << std::endl;

    if (nativeBonusApplied)
    {
        std::cout << "Bonus natif : actif." << std::endl;
    }

    std::cout << std::endl;

    mainPlayer.displayStats();
    mainPlayer.displaySimpleEquipment();

    saveCurrentProgress("Création du personnage");

    Console::waitForEnter();
    Console::clear();
}

// EN: chooseGameMode declares or implements a focused behavior used by this module.
// FR: chooseGameMode déclare ou implémente un comportement précis utilisé par ce module.
void Game::chooseGameMode()
{
    while (true)
    {
        std::cout << "========== ACTIVITÉS ==========" << std::endl;
        std::cout << "1 : Histoire" << std::endl;
        std::cout << "    Mode aventure principal. Il reste volontairement en attente pour la fin du développement." << std::endl;
        std::cout << std::endl;

        std::cout << "2 : Combats" << std::endl;
        std::cout << "    PvE monstres, boss, groupes d'adversaires et JcJ." << std::endl;
        std::cout << std::endl;

        std::cout << "3 : Exploration" << std::endl;
        std::cout << "    Recherche passive par biome : plantes, matériaux, coffres, pièges, mimics et événements." << std::endl;
        std::cout << std::endl;

        std::cout << "4 : Quêtes" << std::endl;
        std::cout << "    Guilde, journal, demandes de PNJ et progression de quêtes." << std::endl;
        std::cout << std::endl;

        std::cout << "5 : Boutiques / lieux visitables" << std::endl;
        std::cout << "    Forge, herboristerie, bibliothèque, boutiques spécialisées et lieux sociaux." << std::endl;
        std::cout << std::endl;

        std::cout << "6 : PNJ notables" << std::endl;
        std::cout << "    Parler aux clients et personnages disponibles sans passer par une boutique." << std::endl;
        std::cout << std::endl;

        std::cout << "7 : Échange / don" << std::endl;
        std::cout << "    Transférer des ressources entre personnages compatibles." << std::endl;
        std::cout << std::endl;

        std::cout << "8 : Information sur toutes les options" << std::endl;
        std::cout << "    Explique concrètement ce que fait chaque activité." << std::endl;
        std::cout << std::endl;

        std::cout << "0 : Sauvegarder et quitter" << std::endl;
        std::cout << "================================" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(
            0,
            8,
            "Veuillez entrer un chiffre valide entre 0 et 8."
        );

        Console::clear();

        if (choice == 0)
        {
            saveCurrentProgress("Sauvegarder et quitter");
            std::cout << "Progression sauvegardée. Fermeture de Dinotofu." << std::endl;
            std::cout << std::endl;
            std::exit(0);
        }

        if (choice == 8)
        {
            displayActivityInformation();
            continue;
        }

        if (choice == 2)
        {
            std::cout << "========== COMBATS ==========" << std::endl;
            std::cout << "0 : Retour" << std::endl;
            std::cout << "1 : PvP IA" << std::endl;
            std::cout << "    Duel contre une IA, avec personnages spéciaux possibles selon le mode." << std::endl;
            std::cout << "2 : PvP 2 joueurs / JcJ" << std::endl;
            std::cout << "    Duel local amical ou mortel selon les comptes, clones, altérations et difficultés." << std::endl;
            std::cout << "3 : PvE monstres" << std::endl;
            std::cout << "    Vagues de monstres, loots, matériaux, qualité de récupération et progression." << std::endl;
            std::cout << "4 : PvE Boss" << std::endl;
            std::cout << "    Combat contre un boss avec identité, décryptage et fragments spéciaux." << std::endl;
            std::cout << "============================" << std::endl;
            std::cout << "> ";

            int combatChoice = Console::askNumberBetween(0, 4, "Veuillez choisir un combat affiché.");
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

    std::cout << "Activité sélectionnée : ";

    switch (selectedMode)
    {
        case GameMode::Story:
            std::cout << "Histoire";
            break;

        case GameMode::AIPvp:
            std::cout << "Combat - PvP IA";
            break;

        case GameMode::TwoPlayerPvp:
            std::cout << "Combat - PvP 2 joueurs / JcJ";
            break;

        case GameMode::MonsterPve:
            std::cout << "Combat - PvE monstres";
            break;

        case GameMode::BossPve:
            std::cout << "Combat - PvE Boss";
            break;

        case GameMode::Challenges:
            std::cout << "Quêtes";
            break;

        case GameMode::Exploration:
            std::cout << "Exploration";
            break;

        case GameMode::Locations:
            std::cout << "Boutiques / lieux visitables";
            break;

        case GameMode::NotableNpcs:
            std::cout << "PNJ notables";
            break;

        case GameMode::Exchange:
            std::cout << "Échange / don";
            break;
    }

    std::cout << std::endl;
    std::cout << "Difficulté : " << getDifficultyName() << std::endl;
    std::cout << std::endl;

    Console::waitForEnter();
    Console::clear();
}

// EN: displayActivityInformation declares or implements a focused behavior used by this module.
// FR: displayActivityInformation déclare ou implémente un comportement précis utilisé par ce module.
void Game::displayActivityInformation() const
{
    std::cout << "========== INFORMATION SUR LES ACTIVITÉS ==========" << std::endl;
    std::cout << "Histoire : mode aventure principal. Il sera développé en dernier, après les systèmes DND et l'interface graphique." << std::endl;
    std::cout << std::endl;

    std::cout << "Combats : regroupe les affrontements volontaires." << std::endl;
    std::cout << "- PvP IA : duel contre une IA ou un adversaire spécial." << std::endl;
    std::cout << "- PvP 2 joueurs / JcJ : duel local entre deux personnages, avec règles amicales ou mortelles." << std::endl;
    std::cout << "- PvE monstres : vagues d'ennemis, loots, matériaux et progression de quêtes." << std::endl;
    std::cout << "- PvE Boss : boss avec phases, identité, fragments et récompenses spéciales." << std::endl;
    std::cout << std::endl;

    std::cout << "Exploration : sortie passive par biome." << std::endl;
    std::cout << "Tu choisis un biome, puis tu peux trouver plantes, matériaux, trésors, coffres, pièges, mimics ou combats inattendus." << std::endl;
    std::cout << "Ce n'est pas un combat volontaire : c'est une activité de recherche et d'événements." << std::endl;
    std::cout << std::endl;

    std::cout << "Quêtes : journal, guilde, demandes de PNJ, quêtes terminées et validation des objectifs." << std::endl;
    std::cout << "Les quêtes de guilde sont limitées, les demandes personnelles peuvent être plus nombreuses." << std::endl;
    std::cout << std::endl;

    std::cout << "Boutiques / lieux visitables : accès aux lieux sociaux comme forge, herboristerie, bibliothèque et vendeurs." << std::endl;
    std::cout << "Certains vendeurs peuvent aussi proposer des demandes personnelles." << std::endl;
    std::cout << std::endl;

    std::cout << "PNJ notables : liste directe des PNJ importants ou clients disponibles." << std::endl;
    std::cout << "Utile pour rendre une demande sans fouiller tous les lieux." << std::endl;
    std::cout << std::endl;

    std::cout << "Échange / don : transfère des objets ou de l'or entre personnages compatibles." << std::endl;
    std::cout << "Les clones, personnages altérés et mélanges Léthal/non-Léthal restent protégés contre les abus." << std::endl;
    std::cout << std::endl;

    std::cout << "Roadmap : l'interface graphique viendra après les systèmes DND. L'histoire complète viendra encore après." << std::endl;
    std::cout << "====================================================" << std::endl;
    std::cout << std::endl;

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
        QuestMenu::openExploration(mainPlayer);
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
        mainPlayer.recordCombatStarted();

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
                combat.launchMonsterPve(mainPlayer, selectedDifficulty);
                break;
            }

            case GameMode::BossPve:
            {
                combat.launchBossPve(mainPlayer, selectedDifficulty);
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

        ShopRotationSystem::markShopsDirtyAfterCombat();

        if (mainPlayer.isDead() && DifficultyRules::isPermanentDeath(selectedDifficulty))
        {
            saveCurrentProgress("Mort définitive");

            if (SaveManager::movePlayableCharacterToDead(accountName, mainPlayer.getName()))
            {
                std::cout << "Le personnage a été déplacé dans le registre des morts." << std::endl;
                std::cout << "Il ne sera plus disponible dans les personnages jouables." << std::endl;
            }
            else
            {
                std::cout << "Impossible de déplacer automatiquement le personnage dans le registre des morts." << std::endl;
                std::cout << "La sauvegarde de mort a tout de même été tentée." << std::endl;
            }

            std::cout << std::endl;
            DeathPenaltySystem::displayLethalDeathCorruption();
            Console::waitForEnter();
            return;
        }

        saveCurrentProgress("Fin de combat");
        QuestMenu::maybeOfferRandomInterception(mainPlayer);
        saveCurrentProgress("Événement de quête éventuel");
    }

    bool continuePlaying = openPostCombatMenu();

    if (continuePlaying)
    {
        chooseGameMode();
        displaySelectedMode();
        launchSelectedMode();
        return;
    }

    saveCurrentProgress("Fin de session");

    std::cout << std::endl;
}

// EN: launchStoryModePlaceholder declares or implements a focused behavior used by this module.
// FR: launchStoryModePlaceholder déclare ou implémente un comportement précis utilisé par ce module.
void Game::launchStoryModePlaceholder()
{
    std::cout << "========== HISTOIRE ==========" << std::endl;
    std::cout << "Le mode Histoire est maintenant placé tout en haut du jeu." << std::endl;
    std::cout << "Pour l'instant, l'aventure principale n'est pas encore développée." << std::endl;
    std::cout << std::endl;
    std::cout << "Contexte prévu :" << std::endl;
    std::cout << "Tu n'es pas seulement un combattant d'arène. Tu es un personnage inscrit dans un monde" << std::endl;
    std::cout << "où les quêtes, les guildes, les boss, les matériaux rares et les choix finiront par compter." << std::endl;
    std::cout << std::endl;
    std::cout << "Pour l'instant, utilise les Quêtes, l'Exploration, le PvE et les Boss pour faire progresser ton personnage." << std::endl;
    std::cout << "Roadmap : les attributs DND seront stabilisés avant l'interface graphique, puis l'histoire complète viendra après." << std::endl;
    std::cout << "==============================" << std::endl;
    std::cout << std::endl;

    Console::waitForEnter();
    Console::clear();
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
        int maxChoice = mainPlayer.isAlteredByCheats() ? 12 : 11;

        std::cout << "========== APRÈS-COMBAT ==========" << std::endl;
        std::cout << "0 : Continuer" << std::endl;
        std::cout << "1 : Ouvrir les boutiques" << std::endl;
        std::cout << "2 : Voir mes statistiques" << std::endl;
        std::cout << "3 : Voir mon équipement" << std::endl;
        std::cout << "4 : Améliorer mes attributs" << std::endl;
        std::cout << "5 : Sauvegarde rapide" << std::endl;
        std::cout << "6 : Sauvegarder et quitter" << std::endl;
        std::cout << "7 : Consulter mes quêtes" << std::endl;
        std::cout << "8 : Lieux visitables" << std::endl;
        std::cout << "9 : PNJ notables" << std::endl;
        std::cout << "10 : Échange / don entre personnages" << std::endl;
        std::cout << "11 : Voir mes compétences" << std::endl;

        if (mainPlayer.isAlteredByCheats())
        {
            std::cout << "12 : Données altérées" << std::endl;
        }

        std::cout << "==================================" << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

        std::string input;
        std::getline(std::cin >> std::ws, input);

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
                std::cout << "Entrée invalide." << std::endl;
                std::cout << std::endl;
            }

            continue;
        }

        Console::clear();

        if (choice < 0 || choice > maxChoice)
        {
            std::cout << "Veuillez choisir une option affichée." << std::endl;
            std::cout << std::endl;
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
            mainPlayer.displaySimpleEquipment();
            Console::waitForEnter();
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
            std::cout << "Progression sauvegardée. Fermeture de Dinotofu." << std::endl;
            std::cout << std::endl;
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
        else if (choice == 12 && mainPlayer.isAlteredByCheats())
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
        std::cout << "Aucun autre compte disponible pour un échange." << std::endl;
        std::cout << std::endl;
        Console::waitForEnter();
        Console::clear();
        return;
    }

    std::cout << "========== ÉCHANGE / DON ==========" << std::endl;
    std::cout << "Choisis le compte cible." << std::endl;
    std::cout << "0 : Retour" << std::endl;

    for (int i = 0; i < static_cast<int>(accounts.size()); ++i)
    {
        std::cout << (i + 1) << " : " << accounts[i].accountName << std::endl;
    }

    std::cout << "===================================" << std::endl;
    std::cout << "> ";

    int accountChoice = Console::askNumberBetween(
        0,
        static_cast<int>(accounts.size()),
        "Veuillez choisir un compte affiché."
    );

    if (accountChoice == 0)
    {
        Console::clear();
        return;
    }

    std::string targetAccount = accounts[accountChoice - 1].accountName;
    std::vector<CharacterSaveSummary> characters = SaveManager::listPlayableCharacters(targetAccount);

    Console::clear();

    if (characters.empty())
    {
        std::cout << "Ce compte n'a aucun personnage jouable." << std::endl;
        std::cout << std::endl;
        Console::waitForEnter();
        Console::clear();
        return;
    }

    std::cout << "========== PERSONNAGE CIBLE ==========" << std::endl;
    std::cout << "0 : Retour" << std::endl;

    for (int i = 0; i < static_cast<int>(characters.size()); ++i)
    {
        std::cout << (i + 1) << " : "
                  << characters[i].characterName
                  << " | " << characters[i].raceName
                  << " / " << characters[i].className
                  << " | Niveau " << characters[i].level
                  << std::endl;
    }

    std::cout << "======================================" << std::endl;
    std::cout << "> ";

    int characterChoice = Console::askNumberBetween(
        0,
        static_cast<int>(characters.size()),
        "Veuillez choisir un personnage affiché."
    );

    if (characterChoice == 0)
    {
        Console::clear();
        return;
    }

    CharacterSaveSummary targetSummary = characters[characterChoice - 1];

    if (targetAccount == accountName && targetSummary.characterName == mainPlayer.getName())
    {
        Console::clear();
        std::cout << "Tu ne peux pas échanger avec le même personnage." << std::endl;
        std::cout << std::endl;
        Console::waitForEnter();
        Console::clear();
        return;
    }

    Player targetPlayer;
    DifficultyMode targetDifficulty = DifficultyMode::Normal;

    if (!SaveManager::loadPlayerSnapshot(targetSummary, targetPlayer, targetDifficulty))
    {
        Console::clear();
        std::cout << "Impossible de charger le personnage cible." << std::endl;
        std::cout << std::endl;
        Console::waitForEnter();
        Console::clear();
        return;
    }

    if (mainPlayer.isAlteredByCheats() || targetPlayer.isAlteredByCheats())
    {
        Console::clear();
        std::cout << "Échange impossible." << std::endl;
        std::cout << "Un personnage altéré ne peut pas transférer de ressources réelles." << std::endl;
        std::cout << "Suggestion sécurité : les données instables restent isolées du commerce entre personnages." << std::endl;
        std::cout << std::endl;
        Console::waitForEnter();
        Console::clear();
        return;
    }

    if (mainPlayer.isClone() || targetPlayer.isClone())
    {
        Console::clear();
        std::cout << "Échange impossible." << std::endl;
        std::cout << "Un clone ne peut pas donner ou recevoir d'objets réels." << std::endl;
        std::cout << "Raison : éviter la duplication par export/copie de personnage." << std::endl;
        std::cout << std::endl;
        Console::waitForEnter();
        Console::clear();
        return;
    }

    bool currentIsLethal = selectedDifficulty == DifficultyMode::Lethal;
    bool targetIsLethal = targetDifficulty == DifficultyMode::Lethal;

    if (currentIsLethal != targetIsLethal)
    {
        Console::clear();
        std::cout << "Échange impossible." << std::endl;
        std::cout << "Un personnage Léthal est considéré comme une vraie existence." << std::endl;
        std::cout << "Un personnage non Léthal reste une simulation plus sûre." << std::endl;
        std::cout << "Pour éviter les abus, il faut deux Léthal ou deux non Léthal." << std::endl;
        std::cout << std::endl;
        Console::waitForEnter();
        Console::clear();
        return;
    }

    bool open = true;

    while (open)
    {
        Console::clear();
        std::cout << "========== ÉCHANGE / DON ==========" << std::endl;
        std::cout << "Source principale : " << mainPlayer.getName() << std::endl;
        std::cout << "Cible : " << targetPlayer.getName() << " (" << targetAccount << ")" << std::endl;
        displayExchangeValueEstimation(mainPlayer, targetPlayer);
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Donner de l'or" << std::endl;
        std::cout << "2 : Donner une arme" << std::endl;
        std::cout << "3 : Donner une armure" << std::endl;
        std::cout << "4 : Donner un consommable" << std::endl;
        std::cout << "5 : Donner un matériau" << std::endl;
        std::cout << "6 : Recevoir depuis le personnage cible" << std::endl;
        std::cout << "===================================" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(0, 6, "Veuillez choisir une option affichée.");

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
            std::cout << "Recevoir depuis " << giver->getName() << " vers " << receiver->getName() << "." << std::endl;
            std::cout << "1 : Or" << std::endl;
            std::cout << "2 : Arme" << std::endl;
            std::cout << "3 : Armure" << std::endl;
            std::cout << "4 : Consommable" << std::endl;
            std::cout << "5 : Matériau" << std::endl;
            std::cout << "> ";
            choice = Console::askNumberBetween(1, 5, "Veuillez choisir une option affichée.");
        }

        Console::clear();

        if (choice == 1)
        {
            std::cout << giver->getName() << " possède " << giver->getInventory().getGold() << " or." << std::endl;
            std::cout << "Montant à transférer ?" << std::endl;
            std::cout << "> ";
            int amount = Console::askNumberBetween(0, giver->getInventory().getGold(), "Montant invalide.");

            if (amount > 0 && giver->getInventory().spendGold(amount))
            {
                receiver->getInventory().earnGold(amount);
                std::cout << amount << " or transféré." << std::endl;
            }
            else
            {
                std::cout << "Aucun or transféré." << std::endl;
            }
        }
        else if (choice == 2)
        {
            giver->getInventory().displayWeaponList();
            std::cout << "Choisis l'arme à transférer. 0 pour annuler." << std::endl;
            std::cout << "> ";
            int index = Console::askNumberBetween(0, giver->getInventory().getWeaponCount(), "Choix invalide.") - 1;

            if (index >= 0)
            {
                if (index == giver->getEquippedWeaponIndex())
                {
                    std::cout << "Impossible de transférer l'arme équipée pour l'instant." << std::endl;
                }
                else
                {
                    Weapon weapon = giver->getInventory().getWeapon(index);
                    receiver->getInventory().addWeapon(weapon);
                    giver->getInventory().removeWeapon(index);
                    std::cout << "Arme transférée : " << weapon.getName() << "." << std::endl;
                }
            }
        }
        else if (choice == 3)
        {
            giver->getInventory().displayArmorList();
            std::cout << "Choisis l'armure à transférer. 0 pour annuler." << std::endl;
            std::cout << "> ";
            int index = Console::askNumberBetween(0, giver->getInventory().getArmorCount(), "Choix invalide.") - 1;

            if (index >= 0)
            {
                if (index == giver->getEquippedArmorIndex())
                {
                    std::cout << "Impossible de transférer l'armure équipée pour l'instant." << std::endl;
                }
                else
                {
                    Armor armor = giver->getInventory().getArmor(index);
                    receiver->getInventory().addArmor(armor);
                    giver->getInventory().removeArmor(index);
                    std::cout << "Armure transférée : " << armor.getName() << "." << std::endl;
                }
            }
        }
        else if (choice == 4)
        {
            giver->getInventory().displayConsumableList();
            std::cout << "Choisis le consommable à transférer. 0 pour annuler." << std::endl;
            std::cout << "> ";
            int index = Console::askNumberBetween(0, giver->getInventory().getConsumableCount(), "Choix invalide.") - 1;

            if (index >= 0)
            {
                Consumable consumable = giver->getInventory().getConsumable(index);
                receiver->getInventory().addConsumable(consumable);
                giver->getInventory().removeConsumable(index);
                std::cout << "Consommable transféré : " << consumable.getName() << "." << std::endl;
            }
        }
        else if (choice == 5)
        {
            giver->getInventory().displayMaterialList();
            std::cout << "Choisis le matériau à transférer. 0 pour annuler." << std::endl;
            std::cout << "> ";
            int index = Console::askNumberBetween(0, static_cast<int>(giver->getInventory().getMaterials().size()), "Choix invalide.") - 1;

            if (index >= 0)
            {
                Material material = giver->getInventory().getMaterial(index);
                std::cout << "Quantité à transférer ? Max : " << material.getQuantity() << std::endl;
                std::cout << "> ";
                int amount = Console::askNumberBetween(1, material.getQuantity(), "Quantité invalide.");
                material.setQuantity(amount);
                receiver->getInventory().addMaterial(material);
                giver->getInventory().removeMaterialQuantity(index, amount);
                std::cout << "Matériau transféré : " << material.getName() << " x" << amount << "." << std::endl;
            }
        }

        SaveManager::savePlayerSnapshot(mainPlayer, accountName, selectedDifficulty);
        SaveManager::savePlayerSnapshot(targetPlayer, targetAccount, targetDifficulty);

        std::cout << std::endl;
        std::cout << "Échange sauvegardé." << std::endl;
        std::cout << std::endl;
        Console::waitForEnter();
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
        std::cout << "Sauvegarde préparée : " << reason << "." << std::endl;
        std::cout << "Chemin : "
                  << SaveManager::getCharacterSavePath(accountName, mainPlayer.getName())
                  << std::endl;
        std::cout << std::endl;
    }
    else
    {
        std::cout << "Sauvegarde impossible pour le moment." << std::endl;
        std::cout << std::endl;
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
