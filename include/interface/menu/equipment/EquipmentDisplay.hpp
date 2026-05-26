// EN: EquipmentDisplay.hpp exposes equipment screens through shared UI models.
// FR: EquipmentDisplay.hpp expose les écrans d'équipement via les modèles d'interface partagés.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_EQUIPMENT_EQUIPMENTDISPLAY_HPP
#define INCLUDE_INTERFACE_MENU_EQUIPMENT_EQUIPMENTDISPLAY_HPP

#include "entity/Player.hpp"
#include "interface/model/MenuScreen.hpp"

#include "item/weapon/Weapon.hpp"
#include "item/armor/Armor.hpp"

#include <string>

class EquipmentDisplay
{
public:
    static MenuScreen buildMainScreen();
    static MenuScreen buildWeaponListScreen(const Player& player);
    static MenuScreen buildArmorListScreen(const Player& player);
    static MenuScreen buildSelectedWeaponScreen(const Weapon& weapon);
    static MenuScreen buildSelectedArmorScreen(const Armor& armor);

    static void displayMainMenu();
    static void displayWeaponList(const Player& player);
    static void displayArmorList(const Player& player);
    static void displaySelectedWeapon(const Weapon& weapon);
    static void displaySelectedArmor(const Armor& armor);

    static std::string weaponSummaryText(const Weapon& weapon, int index);
    static std::string armorSummaryText(const Armor& armor, int index);
    static void displayWeaponSummary(const Weapon& weapon, int index);
    static void displayArmorSummary(const Armor& armor, int index);

    static std::string weaponDurabilityText(const Weapon& weapon);
    static std::string armorDurabilityText(const Armor& armor);
};

#endif
