// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/CombatMenu.hpp"

#include <iostream>

void CombatMenu::displayTurnMenu(const Entity& entity)
{
    std::cout << "========== MENU COMBAT ==========" << std::endl;
    std::cout << "Tour de " << entity.getName() << std::endl;
    std::cout << "---------------------------------" << std::endl;
    std::cout << "0 : Interface" << std::endl;
    std::cout << "1 : Attaquer" << std::endl;
    std::cout << "2 : Potion de soin rapide" << std::endl;
    std::cout << "3 : Potions" << std::endl;
    std::cout << "4 : Équipement" << std::endl;
    std::cout << "5 : Inventaire" << std::endl;
    std::cout << "6 : Passer son tour" << std::endl;
    std::cout << "7 : Fuir" << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void CombatMenu::displayUnavailableOption()
{
    std::cout << "[cette option n'est pas encore accessible pour ce mode]" << std::endl;
    std::cout << std::endl;
}