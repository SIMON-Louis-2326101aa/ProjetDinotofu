// EN: ShopInventory.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: ShopInventory.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    // EN: ShopInventory declares or implements a focused behavior used by this module.
    // FR: ShopInventory déclare ou implémente un comportement précis utilisé par ce module.
    ShopInventory();

    ShopInventory(
        ShopType type,
        const std::string& name
    );

    // EN: getType declares or implements a focused behavior used by this module.
    // FR: getType déclare ou implémente un comportement précis utilisé par ce module.
    ShopType getType() const;
    std::string getName() const;
    // EN: getItems declares or implements a focused behavior used by this module.
    // FR: getItems déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<ShopItem>& getItems() const;
    std::vector<ShopItem>& getMutableItems();

    // EN: addItem declares or implements a focused behavior used by this module.
    // FR: addItem déclare ou implémente un comportement précis utilisé par ce module.
    void addItem(const ShopItem& item);
    // EN: isEmpty declares or implements a focused behavior used by this module.
    // FR: isEmpty déclare ou implémente un comportement précis utilisé par ce module.
    bool isEmpty() const;
};

#endif
