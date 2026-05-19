// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Implements the current rotating inventory of one shop.
// Français : Implémente l'inventaire actuel renouvelable d'une boutique.

#include "economy/shop/ShopInventory.hpp"

ShopInventory::ShopInventory()
{
    type = ShopType::Unknown;
    name = "Boutique inconnue";
}

ShopInventory::ShopInventory(
    ShopType type,
    const std::string& name
)
{
    this->type = type;
    this->name = name;
}

ShopType ShopInventory::getType() const
{
    return type;
}

std::string ShopInventory::getName() const
{
    return name;
}

const std::vector<ShopItem>& ShopInventory::getItems() const
{
    return items;
}

void ShopInventory::addItem(const ShopItem& item)
{
    items.push_back(item);
}

bool ShopInventory::isEmpty() const
{
    return items.empty();
}
