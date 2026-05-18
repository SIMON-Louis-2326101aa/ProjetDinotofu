// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_INVENTORY_INVENTORYDISPLAY_HPP
#define INCLUDE_INTERFACE_MENU_INVENTORY_INVENTORYDISPLAY_HPP

#include "entity/Player.hpp"

#include "item/weapon/Weapon.hpp"
#include "item/armor/Armor.hpp"
#include "item/consumable/Consumable.hpp"

class InventoryDisplay
{
public:
    static void displayMainMenu();

    static void displaySimpleFullInventory(const Player& player);

    static void displaySelectedWeapon(const Weapon& weapon);
    static void displaySelectedArmor(const Armor& armor);
    static void displaySelectedConsumable(const Consumable& consumable);

    static void displayUnavailableMaterials();
};

#endif
