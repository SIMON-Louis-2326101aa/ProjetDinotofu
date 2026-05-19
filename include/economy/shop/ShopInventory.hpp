// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Stores the current rotating inventory of one shop.
// Français : Stocke l'inventaire actuel renouvelable d'une boutique.

#ifndef INCLUDE_ECONOMY_SHOP_SHOPINVENTORY_HPP
#define INCLUDE_ECONOMY_SHOP_SHOPINVENTORY_HPP

#include "economy/shop/ShopItem.hpp"
#include "economy/shop/ShopType.hpp"

#include <string>
#include <vector>

class ShopInventory
{
private:
    ShopType type;
    std::string name;
    std::vector<ShopItem> items;

public:
    ShopInventory();

    ShopInventory(
        ShopType type,
        const std::string& name
    );

    ShopType getType() const;
    std::string getName() const;
    const std::vector<ShopItem>& getItems() const;

    void addItem(const ShopItem& item);
    bool isEmpty() const;
};

#endif
