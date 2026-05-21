// EN: ShopMenu.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: ShopMenu.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Displays the shop menu shown between fights and in towns.
// Français : Affiche le menu de boutique visible entre les combats et dans les villes.

#ifndef INCLUDE_INTERFACE_MENU_SHOP_SHOPMENU_HPP
#define INCLUDE_INTERFACE_MENU_SHOP_SHOPMENU_HPP

#include "entity/Player.hpp"

class ShopMenu
{
public:
    // EN: displayPreview declares or implements a focused behavior used by this module.
    // FR: displayPreview déclare ou implémente un comportement précis utilisé par ce module.
    static void displayPreview();
    // EN: open declares or implements a focused behavior used by this module.
    // FR: open déclare ou implémente un comportement précis utilisé par ce module.
    static void open(Player& player);
};

#endif
