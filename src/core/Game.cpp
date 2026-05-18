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
    selectedMode = GameMode::PvPIA;
}

void Game::run()
{
    Console::clear();

    displayIntroduction();
    askPlayerName();
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

void Game::choosePlayerClass()
{
    std::cout << "Choisis ta classe et entre dans l'arène." << std::endl;
    std::cout << "Trois voies s'offrent à toi :" << std::endl;
    std::cout << std::endl;

    ClassCatalog::displayBaseClasses();

    std::cout << "Veuillez entrer uniquement le chiffre correspondant." << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(
        1,
        3,
        "Veuillez entrer un chiffre valide : 1, 2 ou 3."
    );

    PlayerClass chosenClass = ClassCatalog::creerClasseDeBase(choice);
    mainPlayer = Player(playerName, chosenClass);

    mainPlayer.initializeStarterInventory();

    Console::clear();

    std::cout << playerName << ", tu as choisi la classe : " << chosenClass.getName() << "." << std::endl;
    std::cout << "Tes statistiques ont été gravées dans l'arène avec succès." << std::endl;
    std::cout << "Quelques objets de départ ont été ajoutés à ton inventaire." << std::endl;
    std::cout << std::endl;

    mainPlayer.displayStats();

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

    std::cout << "3 : PvE" << std::endl;
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
        case GameMode::PvPIA:
            std::cout << "PvP IA";
            break;

        case GameMode::PvPDeuxJoueurs:
            std::cout << "PvP 2 joueurs";
            break;

        case GameMode::PvE:
            std::cout << "PvE";
            break;

        case GameMode::PvEBoss:
            std::cout << "PvE Boss";
            break;
    }

    std::cout << std::endl;
    std::cout << std::endl;

    Console::waitForEnter();
    Console::clear();
}

void Game::launchSelectedMode()
{
    Combat combat;

    switch (selectedMode)
    {
        case GameMode::PvPIA:
        {
            combat.launchAiPvp(mainPlayer);
            break;
        }

        case GameMode::PvPDeuxJoueurs:
        {
            combat.launchTwoPlayerPvp(mainPlayer);
            break;
        }

        case GameMode::PvE:
        {
            combat.launchMonsterPve(mainPlayer);
            break;
        }

        case GameMode::PvEBoss:
        {
            combat.launchBossPve(mainPlayer);
            break;
        }
    }

    std::cout << std::endl;
}