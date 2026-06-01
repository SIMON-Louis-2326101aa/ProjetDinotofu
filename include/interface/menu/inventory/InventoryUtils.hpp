// EN: InventoryUtils.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: InventoryUtils.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_INVENTORY_INVENTORYUTILS_HPP
#define INCLUDE_INTERFACE_MENU_INVENTORY_INVENTORYUTILS_HPP

#include "entity/Player.hpp"

#include "item/weapon/Weapon.hpp"
#include "item/armor/Armor.hpp"
#include "item/consumable/ConsumableType.hpp"

#include <string>
#include <vector>

struct ConsumableGroup
{
    int firstIndex;
    int amount;
    std::string name;
    ConsumableType type;
    int power;

    // EN: ConsumableGroup declares or implements a focused behavior used by this module.
    // FR: ConsumableGroup déclare ou implémente un comportement précis utilisé par ce module.
    ConsumableGroup();
};

class InventoryUtils
{
public:
    // EN: consumableTypeToText declares or implements a focused behavior used by this module.
    // FR: consumableTypeToText déclare ou implémente un comportement précis utilisé par ce module.
    static std::string consumableTypeToText(ConsumableType type);

    // EN: weaponDurabilityText declares or implements a focused behavior used by this module.
    // FR: weaponDurabilityText déclare ou implémente un comportement précis utilisé par ce module.
    static std::string weaponDurabilityText(const Weapon& weapon);
    // EN: armorDurabilityText declares or implements a focused behavior used by this module.
    // FR: armorDurabilityText déclare ou implémente un comportement précis utilisé par ce module.
    static std::string armorDurabilityText(const Armor& armor);

    static std::string stackText(int amount);
    static std::string stackLabel(const std::string& name, int amount);

    // EN: groupConsumables declares or implements a focused behavior used by this module.
    // FR: groupConsumables déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<ConsumableGroup> groupConsumables(const Player& player);
};

#endif
