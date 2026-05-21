// EN: InventorySelection.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: InventorySelection.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_INVENTORY_INVENTORYSELECTION_HPP
#define INCLUDE_INTERFACE_MENU_INVENTORY_INVENTORYSELECTION_HPP

#include "entity/Player.hpp"

class InventorySelection
{
public:
    // EN: openWeapons declares or implements a focused behavior used by this module.
    // FR: openWeapons déclare ou implémente un comportement précis utilisé par ce module.
    static bool openWeapons(Player& player);
    // EN: openArmors declares or implements a focused behavior used by this module.
    // FR: openArmors déclare ou implémente un comportement précis utilisé par ce module.
    static bool openArmors(Player& player);
    // EN: openConsumables declares or implements a focused behavior used by this module.
    // FR: openConsumables déclare ou implémente un comportement précis utilisé par ce module.
    static bool openConsumables(Player& player);
    // EN: openMaterials declares or implements a focused behavior used by this module.
    // FR: openMaterials déclare ou implémente un comportement précis utilisé par ce module.
    static bool openMaterials(Player& player);
    // EN: openCraft declares or implements a focused behavior used by this module.
    // FR: openCraft déclare ou implémente un comportement précis utilisé par ce module.
    static bool openCraft(Player& player);
};

#endif
