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
    std::cout << "Choisis ta classe et entre dans l'arène." << std::endl;
    std::cout << std::endl;

    ClassCatalog::displayBasicClasses();

    std::cout << "Veuillez entrer uniquement le chiffre correspondant." << std::endl;
    std::cout << "> ";

    int classChoice = Console::askNumberBetween(
        1,
        3,
        "Veuillez entrer un chiffre valide : 1, 2 ou 3."
    );

    Player player2(player2Name, ClassCatalog::createBaseClass(classChoice));
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