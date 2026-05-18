// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/inventory/InventoryUtils.hpp"

#include "item/Inventory.hpp"
#include "item/consumable/Consumable.hpp"

GroupeConsommable::GroupeConsommable()
    : firstIndex(-1),
      amount(0),
      name(""),
      type(ConsumableType::Unknown),
      power(0)
{
}

std::string InventoryUtils::typeConsommableVersTexte(ConsumableType type)
{
    switch (type)
    {
        case ConsumableType::Healing:
            return "Soin";

        case ConsumableType::Damage:
            return "Offensive";

        case ConsumableType::Buff:
            return "Buff";

        case ConsumableType::Debuff:
            return "Debuff";

        case ConsumableType::Special:
            return "Spéciale";

        default:
            return "Inconnue";
    }
}

std::string InventoryUtils::weaponDurabilityText(const Weapon& weapon)
{
    if (weapon.isIndestructible())
    {
        return "indestructible";
    }

    return std::to_string(weapon.getDurability()) + "/" + std::to_string(weapon.getMaxDurability());
}

std::string InventoryUtils::armorDurabilityText(const Armor& armor)
{
    if (armor.isIndestructible())
    {
        return "indestructible";
    }

    return std::to_string(armor.getDurability()) + "/" + std::to_string(armor.getMaxDurability());
}

std::vector<GroupeConsommable> InventoryUtils::grouperConsommables(const Player& player)
{
    std::vector<GroupeConsommable> groups;
    const std::vector<Consumable>& consumables = player.getInventory().getConsumables();

    for (int i = 0; i < static_cast<int>(consumables.size()); ++i)
    {
        const Consumable& consumable = consumables[i];
        bool trouve = false;

        for (GroupeConsommable& group : groups)
        {
            if (group.name == consumable.getName()
                && group.type == consumable.getType()
                && group.power == consumable.getPower())
            {
                group.amount++;
                trouve = true;
                break;
            }
        }

        if (!trouve)
        {
            GroupeConsommable group;
            group.firstIndex = i;
            group.amount = 1;
            group.name = consumable.getName();
            group.type = consumable.getType();
            group.power = consumable.getPower();

            groups.push_back(group);
        }
    }

    return groups;
}