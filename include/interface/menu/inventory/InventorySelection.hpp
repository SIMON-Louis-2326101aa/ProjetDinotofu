// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_INVENTORY_INVENTORYSELECTION_HPP
#define INCLUDE_INTERFACE_MENU_INVENTORY_INVENTORYSELECTION_HPP

#include "entity/Player.hpp"

class InventorySelection
{
public:
    static bool openWeapons(Player& player);
    static bool openArmors(Player& player);
    static bool openConsumables(Player& player);
    static bool openMaterials(Player& player);
};

#endif
