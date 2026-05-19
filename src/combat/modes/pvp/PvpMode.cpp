// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/modes/pvp/PvpMode.hpp"

#include "combat/TurnManager.hpp"

#include "core/Console.hpp"
#include "class_system/ClassCatalog.hpp"
#include "interface/CombatDisplay.hpp"

#include <iostream>

void PvpMode::run(Player& player1, Random& random)
{
    std::cout << "À votre tour, Joueur 2." << std::endl;
    std::cout << "Quel est ton nom ?" << std::endl;
    std::cout << "> ";

    std::string player2Name;
    std::cin >> player2Name;

    Console::clear();

    std::cout << "Très bien, " << player2Name << "." << std::endl;
    std::cout << "Choisis d'abord ta famille de classe." << std::endl;
    std::cout << std::endl;

    ClassCatalog::displayClassCategories();

    std::cout << std::endl;
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

    Player player2(player2Name, ClassCatalog::createClassByCategoryChoice(categoryChoice, classChoice));
    player2.initializeStarterInventory();

    Console::clear();

    std::cout << player2.getName() << ", tes statistiques ont été gravées dans l'arène." << std::endl;
    std::cout << "Le duel peut maintenant commencer." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(2);

    int turn = random.chooseFirstTurn();

    std::cout << "Préparez-vous..." << std::endl;
    Console::pauseSeconds(2);
    std::cout << "Le combat commence maintenant." << std::endl;
    std::cout << std::endl;

    while (!player1.isDead() && !player2.isDead())
    {
        bool turnFinished = false;

        if (turn == 1)
        {
            turnFinished = TurnManager::playHumanTurn(
                player1,
                player2,
                random,
                POTION_HEAL_AMOUNT,
                POTION_DAMAGE_BONUS
            );

            if (turnFinished)
            {
                turn = 2;
            }
        }
        else
        {
            turnFinished = TurnManager::playHumanTurn(
                player2,
                player1,
                random,
                POTION_HEAL_AMOUNT,
                POTION_DAMAGE_BONUS
            );

            if (turnFinished)
            {
                turn = 1;
            }
        }
    }

    CombatDisplay::displayCombatResult(player1, player2);
}