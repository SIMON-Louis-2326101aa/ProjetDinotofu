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

    ConsumableGroup();
};

class InventoryUtils
{
public:
    static std::string consumableTypeToText(ConsumableType type);

    static std::string weaponDurabilityText(const Weapon& weapon);
    static std::string armorDurabilityText(const Armor& armor);

    static std::vector<ConsumableGroup> groupConsumables(const Player& player);
};

#endif
