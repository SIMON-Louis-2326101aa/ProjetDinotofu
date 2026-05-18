// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "core/Game.hpp"
#include "core/Console.hpp"
#include "class_system/ClassCatalog.hpp"
#include "combat/Combat.hpp"

#include <iostream>
#include <string>

Game::Game()
{
    playerName = "";
    selectedMode = GameMode::AIPvp;
    selectedDifficulty = DifficultyMode::Normal;
}

void Game::run()
{
    Console::clear();

    displayIntroduction();
    askPlayerName();
    chooseDifficulty();
    choosePlayerClass();
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

void Game::askPlayerName()
{
    std::cout << "Quel est ton nom ?" << std::endl;
    std::cout << "> ";

    std::getline(std::cin >> std::ws, playerName);

    while (playerName.empty())
    {
        std::cout << "Un nom vide ? Même les gobelins ont plus de présence que ça." << std::endl;
        std::cout << "Entre un vrai nom." << std::endl;
        std::cout << "> ";

        std::getline(std::cin >> std::ws, playerName);
    }

    Console::clear();

    std::cout << "Très bien, " << playerName << "." << std::endl;
    std::cout << "L'arène se souviendra peut-être de ce nom..." << std::endl;
    std::cout << std::endl;

    Console::waitForEnter();
    Console::clear();
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
    std::cout << std::endl;

    Console::waitForEnter();
    Console::clear();
}

void Game::choosePlayerClass()
{
    std::cout << "Choisis ta classe et entre dans l'arène." << std::endl;
    std::cout << "Trois voies s'offrent à toi :" << std::endl;
    std::cout << std::endl;

    ClassCatalog::displayBasicClasses();

    std::cout << "Veuillez entrer uniquement le chiffre correspondant." << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(
        1,
        3,
        "Veuillez entrer un chiffre valide : 1, 2 ou 3."
    );

    PlayerClass chosenClass = ClassCatalog::createBaseClass(choice);
    mainPlayer = Player(playerName, chosenClass);

    mainPlayer.initializeStarterInventory(selectedDifficulty);

    Console::clear();

    std::cout << playerName << ", tu as choisi la classe : " << chosenClass.getName() << "." << std::endl;
    std::cout << "Difficulté : " << getDifficultyName() << "." << std::endl;
    std::cout << "Tes statistiques ont été gravées dans l'arène avec succès." << std::endl;
    std::cout << "Ton équipement et tes ressources de départ ont été adaptés à la difficulté." << std::endl;
    std::cout << std::endl;

    mainPlayer.displayStats();
    mainPlayer.displaySimpleEquipment();

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

    std::cout << std::endl;
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
