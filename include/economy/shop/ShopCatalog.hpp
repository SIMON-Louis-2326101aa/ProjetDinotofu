// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Creates default shop inventories used before JSON-driven shops are implemented.
// Français : Crée les inventaires de boutiques par défaut avant les boutiques pilotées par JSON.

#ifndef INCLUDE_ECONOMY_SHOP_SHOPCATALOG_HPP
#define INCLUDE_ECONOMY_SHOP_SHOPCATALOG_HPP

#include "economy/shop/ShopInventory.hpp"

#include <vector>

class ShopCatalog
{
public:
    static std::vector<ShopType> getAvailableShopTypes();
    static ShopInventory createPreviewShop(ShopType type);
    static std::vector<ShopInventory> createAllPreviewShops();
};

#endif
