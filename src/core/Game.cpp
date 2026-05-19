// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "core/Game.hpp"
#include "core/Console.hpp"
#include "class_system/ClassCatalog.hpp"
#include "combat/Combat.hpp"
#include "character/RaceCatalog.hpp"
#include "character/SpecialCharacterNativeBonus.hpp"
#include "save/SaveManager.hpp"

#include <iostream>
#include <string>
#include <vector>

Game::Game()
{
    accountName = "local";
    playerName = "";
    selectedMode = GameMode::AIPvp;
    selectedDifficulty = DifficultyMode::Normal;
    selectedRace = CharacterRace::Human;
    characterLoaded = false;
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
        choosePlayerRace();
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
    std::vector<AccountSaveSummary> accounts = SaveManager::listAccounts();

    std::cout << "===== COMPTE LOCAL =====" << std::endl;
    std::cout << "0 : Créer / utiliser un nouveau compte" << std::endl;

    for (int i = 0; i < static_cast<int>(accounts.size()); i++)
    {
        std::cout << (i + 1) << " : " << accounts[i].accountName << std::endl;
    }

    std::cout << "========================" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(
        0,
        static_cast<int>(accounts.size()),
        "Veuillez choisir un compte affiché, ou 0 pour en créer un."
    );

    if (choice == 0)
    {
        std::cout << std::endl;
        std::cout << "Nom du compte local ?" << std::endl;
        std::cout << "Tu peux laisser vide pour utiliser le compte local par défaut." << std::endl;
        std::cout << "> ";

        std::getline(std::cin >> std::ws, accountName);

        if (accountName.empty())
        {
            accountName = "local";
        }
    }
    else
    {
        accountName = accounts[choice - 1].accountName;
    }

    Console::clear();

    std::cout << "Compte actif : " << accountName << "." << std::endl;
    std::cout << "Les sauvegardes sont rangées dans assets/saves/." << std::endl;
    std::cout << std::endl;

    if (!SaveManager::saveAccountSnapshot(accountName))
    {
        std::cout << "Attention : impossible de préparer la sauvegarde du compte pour le moment." << std::endl;
        std::cout << std::endl;
    }

    Console::waitForEnter();
    Console::clear();
}

void Game::askPlayerName()
{
    std::vector<CharacterSaveSummary> characters = SaveManager::listPlayableCharacters(accountName);

    std::cout << "===== PERSONNAGE =====" << std::endl;
    std::cout << "0 : Créer un nouveau personnage" << std::endl;

    for (int i = 0; i < static_cast<int>(characters.size()); i++)
    {
        std::cout << (i + 1) << " : "
                  << characters[i].characterName
                  << " | " << characters[i].raceName
                  << " / " << characters[i].className
                  << " | Niveau " << characters[i].level
                  << std::endl;
    }

    std::cout << "======================" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(
        0,
        static_cast<int>(characters.size()),
        "Veuillez choisir un personnage affiché, ou 0 pour en créer un."
    );

    if (choice > 0)
    {
        if (SaveManager::loadPlayerSnapshot(characters[choice - 1], mainPlayer, selectedDifficulty))
        {
            playerName = mainPlayer.getName();
            selectedRace = mainPlayer.getRace();
            characterLoaded = true;

            Console::clear();

            std::cout << "Personnage chargé : " << playerName << "." << std::endl;
            std::cout << "Race : " << mainPlayer.getRaceText() << std::endl;
            std::cout << "Classe : " << mainPlayer.getType() << std::endl;
            std::cout << "Difficulté : " << getDifficultyName() << std::endl;
            std::cout << std::endl;
            std::cout << "Note : pour le moment, la sauvegarde restaure surtout l'identité,"
                      << " la progression simple, l'or et l'équipement de départ reconstruit." << std::endl;
            std::cout << "L'inventaire complet sera sérialisé proprement plus tard." << std::endl;
            std::cout << std::endl;

            mainPlayer.displayStats();
            mainPlayer.displaySimpleEquipment();

            Console::waitForEnter();
            Console::clear();
            return;
        }

        Console::clear();
        std::cout << "Impossible de charger ce personnage. On va en créer un nouveau." << std::endl;
        std::cout << std::endl;
    }

    characterLoaded = false;

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

    saveCurrentProgress("Fin de session");

    std::cout << std::endl;
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
