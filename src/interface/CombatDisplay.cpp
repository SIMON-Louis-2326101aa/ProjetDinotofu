// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/CombatDisplay.hpp"

#include "character/SpecialCharacterDialogueCatalog.hpp"

#include <iostream>

void CombatDisplay::displayCombatResult(
    const Entity& combattant1,
    const Entity& combattant2
)
{
    std::cout << std::endl;
    std::cout << "========== RÉSULTAT DU COMBAT ==========" << std::endl;

    if (combattant1.isDead() && combattant2.isDead())
    {
        std::cout << "Les deux combattants s'effondrent en même temps." << std::endl;
        std::cout << "L'arène reste silencieuse... aucun vainqueur clair." << std::endl;
    }
    else if (combattant1.isDead())
    {
        std::cout << combattant1.getName() << " tombe au sol." << std::endl;
        std::cout << combattant2.getName() << " remporte le duel." << std::endl;

        SpecialCharacterDialogueCatalog::displayVictoryDialogue(combattant2.getName());
        SpecialCharacterDialogueCatalog::displayDefeatDialogue(combattant1.getName());
    }
    else if (combattant2.isDead())
    {
        std::cout << combattant2.getName() << " tombe au sol." << std::endl;
        std::cout << combattant1.getName() << " remporte le duel." << std::endl;

        SpecialCharacterDialogueCatalog::displayVictoryDialogue(combattant1.getName());
        SpecialCharacterDialogueCatalog::displayDefeatDialogue(combattant2.getName());
    }
    else
    {
        std::cout << "Le combat s'arrête sans vainqueur." << std::endl;
    }

    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
}