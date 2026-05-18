// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/potions/CombatPotionUtils.hpp"

#include "item/Inventory.hpp"
#include "item/consumable/Consumable.hpp"

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