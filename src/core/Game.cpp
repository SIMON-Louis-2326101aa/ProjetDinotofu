// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "core/Game.hpp"
#include "core/Console.hpp"
#include "class_system/ClassCatalog.hpp"
#include "combat/Combat.hpp"
#include "character/RaceCatalog.hpp"
#include "character/SpecialCharacterNativeBonus.hpp"
#include "save/SaveManager.hpp"
#include "save/menu/AccountMenu.hpp"
#include "save/menu/CharacterMenu.hpp"
#include "economy/shop/ShopRotationSystem.hpp"
#include "interface/menu/shop/ShopMenu.hpp"
#include "cheat/CheatManager.hpp"
#include "progression/DifficultyRules.hpp"
#include "progression/death/DeathPenaltySystem.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

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


void Game::askAccountName()
{
    accountName = AccountMenu::open();
}

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

void Game::chooseGameMode()
{
    std::cout << "Sélectionne le mode de jeu que tu veux lancer :" << std::endl;
    std::cout << std::endl;

    std::cout << "1 : PvP IA" << std::endl;
    std::cout << "    Affronte Matt, une IA simple mais prête à te casser les dents." << std::endl;
    std::cout << std::endl;

    std::cout << "2 : PvP 2 joueurs" << std::endl;
    std::cout << "    Deux combattants, une arène, aucune excuse." << std::endl;
    std::cout << std::endl;

    std::cout << "3 : PvE monstres" << std::endl;
    std::cout << "    Une vague d'ennemis avance vers toi. Trois devant, les autres en file." << std::endl;
    std::cout << std::endl;

    std::cout << "4 : PvE Boss" << std::endl;
    std::cout << "    Une entité supérieure t'attend. Mauvaise idée ? Probablement." << std::endl;
    std::cout << std::endl;

    std::cout << "Veuillez entrer uniquement le chiffre correspondant." << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(
        1,
        4,
        "Veuillez entrer un chiffre valide : 1, 2, 3 ou 4."
    );

    selectedMode = static_cast<GameMode>(choice);
}

void Game::displaySelectedMode()
{
    Console::clear();

    std::cout << "Mode sélectionné : ";

    switch (selectedMode)
    {
        case GameMode::AIPvp:
            std::cout << "PvP IA";
            break;

        case GameMode::TwoPlayerPvp:
            std::cout << "PvP 2 joueurs";
            break;

        case GameMode::MonsterPve:
            std::cout << "PvE monstres";
            break;

        case GameMode::BossPve:
            std::cout << "PvE Boss";
            break;
    }

    std::cout << std::endl;
    std::cout << "Difficulté : " << getDifficultyName() << std::endl;
    std::cout << std::endl;

    Console::waitForEnter();
    Console::clear();
}

void Game::launchSelectedMode()
{
    Combat combat;

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
            combat.launchTwoPlayerPvp(mainPlayer);
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

bool Game::openPostCombatMenu()
{
    bool menuOpen = true;

    while (menuOpen)
    {
        int maxChoice = mainPlayer.isAlteredByCheats() ? 6 : 5;

        std::cout << "========== APRÈS-COMBAT ==========" << std::endl;
        std::cout << "0 : Continuer" << std::endl;
        std::cout << "1 : Ouvrir les boutiques" << std::endl;
        std::cout << "2 : Voir mes statistiques" << std::endl;
        std::cout << "3 : Voir mon équipement" << std::endl;
        std::cout << "4 : Sauvegarde rapide" << std::endl;
        std::cout << "5 : Sauvegarder et quitter" << std::endl;

        if (mainPlayer.isAlteredByCheats())
        {
            std::cout << "6 : Données altérées" << std::endl;
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
            mainPlayer.displayStats();
            mainPlayer.displayCareerStatistics(selectedDifficulty);
            Console::waitForEnter();
            Console::clear();
        }
        else if (choice == 3)
        {
            mainPlayer.displaySimpleEquipment();
            Console::waitForEnter();
            Console::clear();
        }
        else if (choice == 4)
        {
            saveCurrentProgress("Sauvegarde rapide");
            Console::waitForEnter();
            Console::clear();
        }
        else if (choice == 5)
        {
            saveCurrentProgress("Sauvegarder et quitter");
            std::cout << "Progression sauvegardée. Fermeture de Dinotofu." << std::endl;
            std::cout << std::endl;
            return false;
        }
        else if (choice == 6 && mainPlayer.isAlteredByCheats())
        {
            CheatManager::openAlteredDataMenu(mainPlayer, selectedDifficulty);
            saveCurrentProgress("Données altérées");
        }
    }

    return false;
}

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
