// EN: ShopItem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: ShopItem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Implements a temporary shop listing entry.
// Français : Implémente une entrée temporaire de vente en boutique.

#include "economy/shop/ShopItem.hpp"

// EN: ShopItem declares or implements a focused behavior used by this module.
// FR: ShopItem déclare ou implémente un comportement précis utilisé par ce module.
ShopItem::ShopItem()
{
    id = "unknown";
    name = "Article inconnu";
    description = "Aucune description.";
    category = ShopItemCategory::Unknown;
    buyPrice = 0;
    sellPrice = 0;
    stock = 0;
    commonInformation = false;
}

ShopItem::ShopItem(
    const std::string& id,
    const std::string& name,
    const std::string& description,
    ShopItemCategory category,
    int buyPrice,
    int sellPrice,
    int stock,
    bool commonInformation
)
{
    this->id = id;
    this->name = name;
    this->description = description;
    this->category = category;
    this->buyPrice = buyPrice;
    this->sellPrice = sellPrice;
    this->stock = stock;
    this->commonInformation = commonInformation;
}

std::string ShopItem::getId() const
{
    return id;
}

std::string ShopItem::getName() const
{
    return name;
}

std::string ShopItem::getDescription() const
{
    return description;
}

// EN: getCategory declares or implements a focused behavior used by this module.
// FR: getCategory déclare ou implémente un comportement précis utilisé par ce module.
ShopItemCategory ShopItem::getCategory() const
{
    return category;
}

// EN: getBuyPrice declares or implements a focused behavior used by this module.
// FR: getBuyPrice déclare ou implémente un comportement précis utilisé par ce module.
int ShopItem::getBuyPrice() const
{
    return buyPrice;
}

// EN: getSellPrice declares or implements a focused behavior used by this module.
// FR: getSellPrice déclare ou implémente un comportement précis utilisé par ce module.
int ShopItem::getSellPrice() const
{
    return sellPrice;
}

// EN: getStock declares or implements a focused behavior used by this module.
// FR: getStock déclare ou implémente un comportement précis utilisé par ce module.
int ShopItem::getStock() const
{
    return stock;
}

// EN: isCommonInformation declares or implements a focused behavior used by this module.
// FR: isCommonInformation déclare ou implémente un comportement précis utilisé par ce module.
bool ShopItem::isCommonInformation() const
{
    return commonInformation;
}

// EN: hasStock declares or implements a focused behavior used by this module.
// FR: hasStock déclare ou implémente un comportement précis utilisé par ce module.
bool ShopItem::hasStock() const
{
    return stock != 0;
}

// EN: isSoldOut declares or implements a focused behavior used by this module.
// FR: isSoldOut déclare ou implémente un comportement précis utilisé par ce module.
bool ShopItem::isSoldOut() const
{
    return stock == 0;
}

// EN: consumeOneStock declares or implements a focused behavior used by this module.
// FR: consumeOneStock déclare ou implémente un comportement précis utilisé par ce module.
bool ShopItem::consumeOneStock()
{
    if (stock == 0)
    {
        return false;
    }

    if (stock > 0)
    {
        stock--;
    }

    return true;
}

void ShopItem::addStock(int amount)
{
    if (amount <= 0 || stock < 0)
    {
        return;
    }
    stock += amount;
}
