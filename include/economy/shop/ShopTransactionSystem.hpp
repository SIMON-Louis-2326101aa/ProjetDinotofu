// EN: ShopTransactionSystem.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: ShopTransactionSystem.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    // EN: canBeBoughtNow declares or implements a focused behavior used by this module.
    // FR: canBeBoughtNow déclare ou implémente un comportement précis utilisé par ce module.
    static bool canBeBoughtNow(const ShopItem& item);

    static bool buyItem(
        Player& player,
        ShopItem& item,
        int finalPrice
    );

    static bool sellInventoryEntry(
        Player& player,
        ShopType shopType,
        int index,
        int finalSellPrice
    );

    static bool sellInventoryEntryQuantity(
        Player& player,
        ShopType shopType,
        int index,
        int finalSellPrice,
        int quantity
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

    static int getMaxSellQuantityForEntry(
        const Player& player,
        ShopType shopType,
        int index
    );

    static void displaySellableEntries(
        const Player& player,
        ShopType shopType
    );

    // EN: displayUnsupportedPurchaseMessage declares or implements a focused behavior used by this module.
    // FR: displayUnsupportedPurchaseMessage déclare ou implémente un comportement précis utilisé par ce module.
    static void displayUnsupportedPurchaseMessage(const ShopItem& item);
};

#endif
