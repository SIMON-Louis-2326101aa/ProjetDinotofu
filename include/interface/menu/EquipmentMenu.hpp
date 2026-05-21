// EN: EquipmentMenu.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: EquipmentMenu.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_EQUIPMENTMENU_HPP
#define INCLUDE_INTERFACE_MENU_EQUIPMENTMENU_HPP

#include "entity/Player.hpp"

class EquipmentMenu
{
public:
    // EN: open declares or implements a focused behavior used by this module.
    // FR: open déclare ou implémente un comportement précis utilisé par ce module.
    static bool open(Player& player);

    // EN: equipWeaponFromInventory declares or implements a focused behavior used by this module.
    // FR: equipWeaponFromInventory déclare ou implémente un comportement précis utilisé par ce module.
    static bool equipWeaponFromInventory(Player& player);
    // EN: equipArmorFromInventory declares or implements a focused behavior used by this module.
    // FR: equipArmorFromInventory déclare ou implémente un comportement précis utilisé par ce module.
    static bool equipArmorFromInventory(Player& player);
};

#endif
