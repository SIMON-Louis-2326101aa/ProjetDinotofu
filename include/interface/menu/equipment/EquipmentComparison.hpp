// EN: EquipmentComparison.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: EquipmentComparison.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_EQUIPMENT_EQUIPMENTCOMPARISON_HPP
#define INCLUDE_INTERFACE_MENU_EQUIPMENT_EQUIPMENTCOMPARISON_HPP

#include "entity/Player.hpp"

#include "item/weapon/Weapon.hpp"
#include "item/armor/Armor.hpp"

class EquipmentComparison
{
public:
    // EN: displayWeaponComparison declares or implements a focused behavior used by this module.
    // FR: displayWeaponComparison déclare ou implémente un comportement précis utilisé par ce module.
    static void displayWeaponComparison(const Player& player, const Weapon& newWeapon);
    // EN: displayArmorComparison declares or implements a focused behavior used by this module.
    // FR: displayArmorComparison déclare ou implémente un comportement précis utilisé par ce module.
    static void displayArmorComparison(const Player& player, const Armor& newArmor);
};

#endif
