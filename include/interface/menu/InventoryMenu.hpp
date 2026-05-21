// EN: InventoryMenu.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: InventoryMenu.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_INVENTORYMENU_HPP
#define INCLUDE_INTERFACE_MENU_INVENTORYMENU_HPP

#include "entity/Player.hpp"

class InventoryMenu
{
public:
    // EN: open declares or implements a focused behavior used by this module.
    // FR: open déclare ou implémente un comportement précis utilisé par ce module.
    static bool open(Player& player);
};

#endif
