// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_EQUIPMENTMENU_HPP
#define INCLUDE_INTERFACE_MENU_EQUIPMENTMENU_HPP

#include "entity/Player.hpp"

class EquipmentMenu
{
public:
    static bool open(Player& player);

    static bool equiperArmeDepuisInventaire(Player& player);
    static bool equiperArmureDepuisInventaire(Player& player);
};

#endif
