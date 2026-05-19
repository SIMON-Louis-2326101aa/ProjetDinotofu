// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Lets the player spend DND-inspired attribute points after leveling up.

#ifndef INCLUDE_INTERFACE_MENU_PROGRESSION_ATTRIBUTEMENU_HPP
#define INCLUDE_INTERFACE_MENU_PROGRESSION_ATTRIBUTEMENU_HPP

#include "entity/Player.hpp"

class AttributeMenu
{
public:
    static void open(Player& player);
};

#endif
