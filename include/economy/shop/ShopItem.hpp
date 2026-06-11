// EN: ShopItem.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: ShopItem.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    // EN: ShopItem declares or implements a focused behavior used by this module.
    // FR: ShopItem déclare ou implémente un comportement précis utilisé par ce module.
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
    // EN: getCategory declares or implements a focused behavior used by this module.
    // FR: getCategory déclare ou implémente un comportement précis utilisé par ce module.
    ShopItemCategory getCategory() const;
    // EN: getBuyPrice declares or implements a focused behavior used by this module.
    // FR: getBuyPrice déclare ou implémente un comportement précis utilisé par ce module.
    int getBuyPrice() const;
    // EN: getSellPrice declares or implements a focused behavior used by this module.
    // FR: getSellPrice déclare ou implémente un comportement précis utilisé par ce module.
    int getSellPrice() const;
    // EN: getStock declares or implements a focused behavior used by this module.
    // FR: getStock déclare ou implémente un comportement précis utilisé par ce module.
    int getStock() const;
    // EN: isCommonInformation declares or implements a focused behavior used by this module.
    // FR: isCommonInformation déclare ou implémente un comportement précis utilisé par ce module.
    bool isCommonInformation() const;
    // EN: hasStock declares or implements a focused behavior used by this module.
    // FR: hasStock déclare ou implémente un comportement précis utilisé par ce module.
    bool hasStock() const;

    // EN: isSoldOut declares or implements a focused behavior used by this module.
    // FR: isSoldOut déclare ou implémente un comportement précis utilisé par ce module.
    bool isSoldOut() const;
    // EN: consumeOneStock declares or implements a focused behavior used by this module.
    // FR: consumeOneStock déclare ou implémente un comportement précis utilisé par ce module.
    bool consumeOneStock();
    void addStock(int amount);
};

#endif
