// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/modes/pvp/AiPvpMode.hpp"

#include "combat/TurnManager.hpp"

#include "core/Console.hpp"
#include "class_system/ClassCatalog.hpp"
#include "interface/CombatDisplay.hpp"

#include <iostream>

void AiPvpMode::run(Player& player1, Random& random)
{
    std::cout << "Préparation de l'IA..." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(1);

    std::cout << "Matt est entré dans l'arène." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(1);

    std::cout << player1.getName() << ", choisis comment Matt obtiendra sa classe :" << std::endl;
    std::cout << std::endl;
    std::cout << "1 : Classe aléatoire" << std::endl;
    std::cout << "2 : Choisir sa classe toi-même" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";

    int choixTypeClasse = Console::askNumberBetween(
        1,
        2,
        "Veuillez entrer un chiffre valide : 1 ou 2."
    );

    int choixClasseIA;

    if (choixTypeClasse == 1)
    {
        choixClasseIA = random.between(1, 3);
    }
    else
    {
        Console::clear();

        std::cout << "Choisis la classe que Matt devra utiliser." << std::endl;
        std::cout << "Pas très fair-play, mais l'arène accepte ce genre de petit caprice." << std::endl;
        std::cout << std::endl;

        ClassCatalog::displayBaseClasses();

        std::cout << "Veuillez entrer uniquement le chiffre correspondant." << std::endl;
        std::cout << "> ";

        choixClasseIA = Console::askNumberBetween(
            1,
            3,
            "Veuillez entrer un chiffre valide : 1, 2 ou 3."
        );
    }

    Player ai("Matt", ClassCatalog::creerClasseDeBase(choixClasseIA));
    ai.initializeStarterInventory();

    Console::clear();

    std::cout << ai.getName() << " a choisi la classe : " << ai.getType() << "." << std::endl;
    std::cout << "Ses statistiques ont été gravées dans l'arène." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(2);

    int turn = random.chooseFirstTurn();

    std::cout << "Prépare-toi..." << std::endl;
    Console::pauseSeconds(2);
    std::cout << "Le combat contre " << ai.getName() << " commence maintenant." << std::endl;
    std::cout << std::endl;

    while (!player1.isDead() && !ai.isDead())
    {
        bool tourTermine = false;

        if (turn == 1)
        {
            tourTermine = TurnManager::playHumanTurn(
                player1,
                ai,
                random,
                SOIN_POTION,
                BONUS_POTION_DEGATS
            );

            if (tourTermine)
            {
                turn = 2;
            }
        }
        else
        {
            tourTermine = TurnManager::playAITurn(
                ai,
                player1,
                random,
                SOIN_POTION,
                BONUS_POTION_DEGATS
            );

            if (tourTermine)
            {
                turn = 1;
            }
        }
    }

    CombatDisplay::displayCombatResult(player1, ai);
}