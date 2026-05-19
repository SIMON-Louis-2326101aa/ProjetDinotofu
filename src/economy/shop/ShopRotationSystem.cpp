// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Tracks whether shop inventories should be refreshed after a combat.
// Français : Suit si les inventaires de boutiques doivent être renouvelés après un combat.

#include "economy/shop/ShopRotationSystem.hpp"

bool ShopRotationSystem::shopsDirty = false;

void ShopRotationSystem::markShopsDirtyAfterCombat()
{
    shopsDirty = true;
}

bool ShopRotationSystem::shouldRefreshShops()
{
    return shopsDirty;
}

void ShopRotationSystem::markShopsRefreshed()
{
    shopsDirty = false;
}
