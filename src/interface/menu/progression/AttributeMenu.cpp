// EN: AttributeMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: AttributeMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Lets the player spend DND-inspired attribute points after leveling up.

#include "interface/menu/progression/AttributeMenu.hpp"

#include "core/Console.hpp"
#include "progression/DndAttributes.hpp"

#include <iostream>

// EN: open declares or implements a focused behavior used by this module.
// FR: open déclare ou implémente un comportement précis utilisé par ce module.
void AttributeMenu::open(Player& player)
{
    bool menuOpen = true;

    while (menuOpen)
    {
        std::cout << "========== ATTRIBUTS ==========" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "Points disponibles : " << player.getUnspentAttributePoints() << std::endl;
        std::cout << std::endl;

        player.displayAttributes();

        for (int choice = 1; choice <= DndAttributes::getChoiceCount(); choice++)
        {
            std::cout << choice << " : " << DndAttributes::getChoiceName(choice) << std::endl;
            std::cout << "    " << DndAttributes::getChoiceDescription(choice) << std::endl;
        }

        std::cout << "===============================" << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(
            0,
            DndAttributes::getChoiceCount(),
            "Veuillez choisir un attribut affiché."
        );

        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (player.getUnspentAttributePoints() <= 0)
        {
            std::cout << "Tu n'as aucun point d'attribut à dépenser pour le moment." << std::endl;
            std::cout << std::endl;
            Console::waitForEnter();
            Console::clear();
            continue;
        }

        if (player.spendAttributePoint(choice))
        {
            std::cout << DndAttributes::getChoiceName(choice) << " augmente." << std::endl;
            std::cout << "Le personnage se rapproche doucement de sa vraie voie." << std::endl;
            std::cout << std::endl;
        }
        else
        {
            std::cout << "Impossible d'améliorer cet attribut." << std::endl;
            std::cout << std::endl;
        }

        Console::waitForEnter();
        Console::clear();
    }
}

// EN: displayLockedDevelopmentMessage declares or implements a focused behavior used by this module.
// FR: displayLockedDevelopmentMessage déclare ou implémente un comportement précis utilisé par ce module.
void AttributeMenu::displayLockedDevelopmentMessage()
{
    std::cout << "Tu sens que ton potentiel pourrait encore grandir..." << std::endl;
    std::cout << "Mais les registres refusent encore de graver ce choix définitivement." << std::endl;
    std::cout << std::endl;
    std::cout << "[les runes restent muettes]" << std::endl;
    std::cout << std::endl;
}
