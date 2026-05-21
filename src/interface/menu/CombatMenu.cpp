// EN: CombatMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/CombatMenu.hpp"

#include "interface/menu/common/MenuFrame.hpp"

#include <iostream>

// EN: displayTurnMenu declares or implements a focused behavior used by this module.
// FR: displayTurnMenu déclare ou implémente un comportement précis utilisé par ce module.
void CombatMenu::displayTurnMenu(const Entity& entity)
{
    MenuFrame::title("COMBAT");
    MenuFrame::subtitle("Tour de " + entity.getName());
    MenuFrame::separator();
    MenuFrame::option(1, "Attaquer");
    MenuFrame::option(2, "Potion de soin rapide");
    MenuFrame::option(3, "Potions");
    MenuFrame::option(4, "Équipement");
    MenuFrame::option(5, "Inventaire / bestiaire");
    MenuFrame::option(6, "Posture de défense");
    MenuFrame::option(7, "Passer son tour");
    MenuFrame::option(8, "Fuir");
    MenuFrame::option(0, "Interface / aide rapide");
    MenuFrame::end();
    MenuFrame::prompt();
}

// EN: displayUnavailableOption declares or implements a focused behavior used by this module.
// FR: displayUnavailableOption déclare ou implémente un comportement précis utilisé par ce module.
void CombatMenu::displayUnavailableOption()
{
    std::cout << "[cette option n'est pas encore accessible pour ce mode]" << std::endl;
    std::cout << std::endl;
}
