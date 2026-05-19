// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Represents a temporary shop listing entry.
// Français : Représente une entrée temporaire de vente en boutique.

#ifndef INCLUDE_ECONOMY_SHOP_SHOPITEM_HPP
#define INCLUDE_ECONOMY_SHOP_SHOPITEM_HPP

#include "economy/shop/ShopItemCategory.hpp"

#include <string>

class ShopItem
{
private:
    std::string id;
    std::string name;
    std::string description;
    ShopItemCategory category;
    int buyPrice;
    int sellPrice;
    int stock;
    bool commonInformation;

public:
    ShopItem();

    ShopItem(
        const std::string& id,
        const std::string& name,
        const std::string& description,
        ShopItemCategory category,
        int buyPrice,
        int sellPrice,
        int stock,
        bool commonInformation = false
    );

    std::string getId() const;
    std::string getName() const;
    std::string getDescription() const;
    ShopItemCategory getCategory() const;
    int getBuyPrice() const;
    int getSellPrice() const;
    int getStock() const;
    bool isCommonInformation() const;
    bool hasStock() const;
};

#endif
