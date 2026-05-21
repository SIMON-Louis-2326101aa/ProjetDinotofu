// EN: CombatMenu.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatMenu.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_COMBATMENU_HPP
#define INCLUDE_INTERFACE_MENU_COMBATMENU_HPP

#include "entity/Entity.hpp"

class CombatMenu
{
public:
    // EN: displayTurnMenu declares or implements a focused behavior used by this module.
    // FR: displayTurnMenu déclare ou implémente un comportement précis utilisé par ce module.
    static void displayTurnMenu(const Entity& entity);
    // EN: displayUnavailableOption declares or implements a focused behavior used by this module.
    // FR: displayUnavailableOption déclare ou implémente un comportement précis utilisé par ce module.
    static void displayUnavailableOption();
};

#endif
