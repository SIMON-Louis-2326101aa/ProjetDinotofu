// EN: ShopCatalog.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: ShopCatalog.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    // EN: getAvailableShopTypes declares or implements a focused behavior used by this module.
    // FR: getAvailableShopTypes déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<ShopType> getAvailableShopTypes();
    // EN: createPreviewShop declares or implements a focused behavior used by this module.
    // FR: createPreviewShop déclare ou implémente un comportement précis utilisé par ce module.
    static ShopInventory createPreviewShop(ShopType type);
    // EN: createAllPreviewShops declares or implements a focused behavior used by this module.
    // FR: createAllPreviewShops déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<ShopInventory> createAllPreviewShops();
};

#endif
