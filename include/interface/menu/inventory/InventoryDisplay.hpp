// EN: InventoryDisplay.hpp exposes inventory screens through shared UI models.
// FR: InventoryDisplay.hpp expose les écrans d'inventaire via les modèles d'interface partagés.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_INVENTORY_INVENTORYDISPLAY_HPP
#define INCLUDE_INTERFACE_MENU_INVENTORY_INVENTORYDISPLAY_HPP

#include "entity/Player.hpp"
#include "interface/model/MenuScreen.hpp"

#include "item/weapon/Weapon.hpp"
#include "item/armor/Armor.hpp"
#include "item/consumable/Consumable.hpp"
#include "item/material/Material.hpp"

class InventoryDisplay
{
public:
    static MenuScreen buildMainScreen();
    static MenuScreen buildSelectedWeaponScreen(const Weapon& weapon);
    static MenuScreen buildSelectedArmorScreen(const Armor& armor);
    static MenuScreen buildSelectedConsumableScreen(const Consumable& consumable, int amount = 1);
    static MenuScreen buildSelectedMaterialScreen(const Material& material);
    static MenuScreen buildUnavailableMaterialsScreen();

    static void displayMainMenu();
    static MenuScreen buildSimpleFullInventoryScreen(const Player& player);
    static void displaySimpleFullInventory(const Player& player);

    static void displaySelectedWeapon(const Weapon& weapon);
    static void displaySelectedArmor(const Armor& armor);
    static void displaySelectedConsumable(const Consumable& consumable);
    static void displaySelectedMaterial(const Material& material);

    static void displayUnavailableMaterials();
};

#endif
