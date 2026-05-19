// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Implements a temporary shop listing entry.
// Français : Implémente une entrée temporaire de vente en boutique.

#include "economy/shop/ShopItem.hpp"

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

ShopItemCategory ShopItem::getCategory() const
{
    return category;
}

int ShopItem::getBuyPrice() const
{
    return buyPrice;
}

int ShopItem::getSellPrice() const
{
    return sellPrice;
}

int ShopItem::getStock() const
{
    return stock;
}

bool ShopItem::isCommonInformation() const
{
    return commonInformation;
}

bool ShopItem::hasStock() const
{
    return stock != 0;
}
