// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Applies shop purchases and safe resale rules to the player inventory.
// Français : Applique les achats de boutique et les règles de revente sécurisée à l'inventaire du joueur.

#ifndef INCLUDE_ECONOMY_SHOP_SHOPTRANSACTIONSYSTEM_HPP
#define INCLUDE_ECONOMY_SHOP_SHOPTRANSACTIONSYSTEM_HPP

#include "economy/shop/ShopItem.hpp"
#include "economy/shop/ShopType.hpp"
#include "entity/Player.hpp"

class ShopTransactionSystem
{
public:
    static bool canBeBoughtNow(const ShopItem& item);

    static bool buyItem(
        Player& player,
        const ShopItem& item,
        int finalPrice
    );

    static bool sellInventoryEntry(
        Player& player,
        ShopType shopType,
        int index,
        int finalSellPrice
    );

    static int getSellPriceForEntry(
        const Player& player,
        ShopType shopType,
        int index
    );

    static bool canShopBuyInventoryEntry(
        const Player& player,
        ShopType shopType,
        int index
    );

    static int getSellableEntryCount(
        const Player& player,
        ShopType shopType
    );

    static void displaySellableEntries(
        const Player& player,
        ShopType shopType
    );

    static void displayUnsupportedPurchaseMessage(const ShopItem& item);
};

#endif
