// EN: CombatPotionUtils.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatPotionUtils.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/potions/CombatPotionUtils.hpp"

#include "item/Inventory.hpp"
#include "item/consumable/Consumable.hpp"

#include <algorithm>

PotionStack::PotionStack()
    : firstIndex(-1),
      amount(0),
      name(""),
      type(ConsumableType::Unknown),
      power(0),
      value(0)
{
}

std::vector<int> CombatPotionUtils::getPotionIndices(
    const Player& player,
    ConsumableType type
)
{
    std::vector<int> indices;

    const std::vector<Consumable>& consumables =
        player.getInventory().getConsumables();

    for (int i = 0; i < static_cast<int>(consumables.size()); ++i)
    {
        if (consumables[i].getType() == type)
        {
            indices.push_back(i);
        }
    }

    return indices;
}

std::vector<PotionStack> CombatPotionUtils::groupPotionIndices(
    const Player& player,
    const std::vector<int>& indices
)
{
    std::vector<PotionStack> stacks;

    for (int inventoryIndex : indices)
    {
        if (!player.getInventory().hasConsumable(inventoryIndex))
        {
            continue;
        }

        Consumable potion = player.getInventory().getConsumable(inventoryIndex);
        bool found = false;

        for (PotionStack& stack : stacks)
        {
            if (stack.name == potion.getName()
                && stack.type == potion.getType()
                && stack.power == potion.getPower()
                && stack.value == potion.getValue())
            {
                stack.amount++;
                found = true;
                break;
            }
        }

        if (!found)
        {
            PotionStack stack;
            stack.firstIndex = inventoryIndex;
            stack.amount = 1;
            stack.name = potion.getName();
            stack.type = potion.getType();
            stack.power = potion.getPower();
            stack.value = potion.getValue();
            stacks.push_back(stack);
        }
    }

    return stacks;
}

std::vector<PotionStack> CombatPotionUtils::groupPotions(const Player& player)
{
    std::vector<int> indices;
    const std::vector<Consumable>& consumables = player.getInventory().getConsumables();

    for (int i = 0; i < static_cast<int>(consumables.size()); ++i)
    {
        indices.push_back(i);
    }

    return groupPotionIndices(player, indices);
}

std::string CombatPotionUtils::stackText(int amount)
{
    return "(*" + std::to_string(std::max(1, amount)) + ")";
}

std::string CombatPotionUtils::stackLabel(const std::string& name, int amount)
{
    return name + " " + stackText(amount);
}

std::string CombatPotionUtils::typeToText(ConsumableType type)
{
    switch (type)
    {
        case ConsumableType::Healing:
            return "Curative";

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
