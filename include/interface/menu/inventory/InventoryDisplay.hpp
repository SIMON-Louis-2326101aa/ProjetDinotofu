// EN: InventoryDisplay.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: InventoryDisplay.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_INVENTORY_INVENTORYDISPLAY_HPP
#define INCLUDE_INTERFACE_MENU_INVENTORY_INVENTORYDISPLAY_HPP

#include "entity/Player.hpp"

#include "item/weapon/Weapon.hpp"
#include "item/armor/Armor.hpp"
#include "item/consumable/Consumable.hpp"
#include "item/material/Material.hpp"

class InventoryDisplay
{
public:
    // EN: displayMainMenu declares or implements a focused behavior used by this module.
    // FR: displayMainMenu déclare ou implémente un comportement précis utilisé par ce module.
    static void displayMainMenu();

    // EN: displaySimpleFullInventory declares or implements a focused behavior used by this module.
    // FR: displaySimpleFullInventory déclare ou implémente un comportement précis utilisé par ce module.
    static void displaySimpleFullInventory(const Player& player);

    // EN: displaySelectedWeapon declares or implements a focused behavior used by this module.
    // FR: displaySelectedWeapon déclare ou implémente un comportement précis utilisé par ce module.
    static void displaySelectedWeapon(const Weapon& weapon);
    // EN: displaySelectedArmor declares or implements a focused behavior used by this module.
    // FR: displaySelectedArmor déclare ou implémente un comportement précis utilisé par ce module.
    static void displaySelectedArmor(const Armor& armor);
    // EN: displaySelectedConsumable declares or implements a focused behavior used by this module.
    // FR: displaySelectedConsumable déclare ou implémente un comportement précis utilisé par ce module.
    static void displaySelectedConsumable(const Consumable& consumable);
    // EN: displaySelectedMaterial declares or implements a focused behavior used by this module.
    // FR: displaySelectedMaterial déclare ou implémente un comportement précis utilisé par ce module.
    static void displaySelectedMaterial(const Material& material);

    // EN: displayUnavailableMaterials declares or implements a focused behavior used by this module.
    // FR: displayUnavailableMaterials déclare ou implémente un comportement précis utilisé par ce module.
    static void displayUnavailableMaterials();
};

#endif
