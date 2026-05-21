// EN: EquipmentDisplay.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: EquipmentDisplay.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_EQUIPMENT_EQUIPMENTDISPLAY_HPP
#define INCLUDE_INTERFACE_MENU_EQUIPMENT_EQUIPMENTDISPLAY_HPP

#include "entity/Player.hpp"

#include "item/weapon/Weapon.hpp"
#include "item/armor/Armor.hpp"

#include <string>

class EquipmentDisplay
{
public:
    // EN: displayMainMenu declares or implements a focused behavior used by this module.
    // FR: displayMainMenu déclare ou implémente un comportement précis utilisé par ce module.
    static void displayMainMenu();

    // EN: displayWeaponList declares or implements a focused behavior used by this module.
    // FR: displayWeaponList déclare ou implémente un comportement précis utilisé par ce module.
    static void displayWeaponList(const Player& player);
    // EN: displayArmorList declares or implements a focused behavior used by this module.
    // FR: displayArmorList déclare ou implémente un comportement précis utilisé par ce module.
    static void displayArmorList(const Player& player);

    // EN: displaySelectedWeapon declares or implements a focused behavior used by this module.
    // FR: displaySelectedWeapon déclare ou implémente un comportement précis utilisé par ce module.
    static void displaySelectedWeapon(const Weapon& weapon);
    // EN: displaySelectedArmor declares or implements a focused behavior used by this module.
    // FR: displaySelectedArmor déclare ou implémente un comportement précis utilisé par ce module.
    static void displaySelectedArmor(const Armor& armor);

    // EN: displayWeaponSummary declares or implements a focused behavior used by this module.
    // FR: displayWeaponSummary déclare ou implémente un comportement précis utilisé par ce module.
    static void displayWeaponSummary(const Weapon& weapon, int index);
    // EN: displayArmorSummary declares or implements a focused behavior used by this module.
    // FR: displayArmorSummary déclare ou implémente un comportement précis utilisé par ce module.
    static void displayArmorSummary(const Armor& armor, int index);

    // EN: weaponDurabilityText declares or implements a focused behavior used by this module.
    // FR: weaponDurabilityText déclare ou implémente un comportement précis utilisé par ce module.
    static std::string weaponDurabilityText(const Weapon& weapon);
    // EN: armorDurabilityText declares or implements a focused behavior used by this module.
    // FR: armorDurabilityText déclare ou implémente un comportement précis utilisé par ce module.
    static std::string armorDurabilityText(const Armor& armor);
};

#endif
