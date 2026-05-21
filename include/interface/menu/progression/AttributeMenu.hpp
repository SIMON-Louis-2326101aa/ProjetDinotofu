// EN: AttributeMenu.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: AttributeMenu.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Lets the player spend DND-inspired attribute points after leveling up.

#ifndef INCLUDE_INTERFACE_MENU_PROGRESSION_ATTRIBUTEMENU_HPP
#define INCLUDE_INTERFACE_MENU_PROGRESSION_ATTRIBUTEMENU_HPP

#include "entity/Player.hpp"

class AttributeMenu
{
public:
    // EN: open declares or implements a focused behavior used by this module.
    // FR: open déclare ou implémente un comportement précis utilisé par ce module.
    static void open(Player& player);
    // EN: displayLockedDevelopmentMessage declares or implements a focused behavior used by this module.
    // FR: displayLockedDevelopmentMessage déclare ou implémente un comportement précis utilisé par ce module.
    static void displayLockedDevelopmentMessage();
};

#endif
