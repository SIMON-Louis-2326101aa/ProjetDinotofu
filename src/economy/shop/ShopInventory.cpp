// EN: ShopInventory.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: ShopInventory.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Implements the current rotating inventory of one shop.
// Français : Implémente l'inventaire actuel renouvelable d'une boutique.

#include "economy/shop/ShopInventory.hpp"

// EN: ShopInventory declares or implements a focused behavior used by this module.
// FR: ShopInventory déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: getType declares or implements a focused behavior used by this module.
// FR: getType déclare ou implémente un comportement précis utilisé par ce module.
ShopType ShopInventory::getType() const
{
    return type;
}

std::string ShopInventory::getName() const
{
    return name;
}

// EN: getItems declares or implements a focused behavior used by this module.
// FR: getItems déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<ShopItem>& ShopInventory::getItems() const
{
    return items;
}

std::vector<ShopItem>& ShopInventory::getMutableItems()
{
    return items;
}

// EN: addItem declares or implements a focused behavior used by this module.
// FR: addItem déclare ou implémente un comportement précis utilisé par ce module.
void ShopInventory::addItem(const ShopItem& item)
{
    items.push_back(item);
}

// EN: isEmpty declares or implements a focused behavior used by this module.
// FR: isEmpty déclare ou implémente un comportement précis utilisé par ce module.
bool ShopInventory::isEmpty() const
{
    return items.empty();
}
