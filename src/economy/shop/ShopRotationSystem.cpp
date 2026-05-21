// EN: ShopRotationSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: ShopRotationSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Tracks whether shop inventories should be refreshed after a combat.
// Français : Suit si les inventaires de boutiques doivent être renouvelés après un combat.

#include "economy/shop/ShopRotationSystem.hpp"

bool ShopRotationSystem::shopsDirty = false;
int ShopRotationSystem::rotationIndex = 0;

// EN: markShopsDirtyAfterCombat declares or implements a focused behavior used by this module.
// FR: markShopsDirtyAfterCombat déclare ou implémente un comportement précis utilisé par ce module.
void ShopRotationSystem::markShopsDirtyAfterCombat()
{
    shopsDirty = true;
}

// EN: shouldRefreshShops declares or implements a focused behavior used by this module.
// FR: shouldRefreshShops déclare ou implémente un comportement précis utilisé par ce module.
bool ShopRotationSystem::shouldRefreshShops()
{
    return shopsDirty;
}

// EN: markShopsRefreshed declares or implements a focused behavior used by this module.
// FR: markShopsRefreshed déclare ou implémente un comportement précis utilisé par ce module.
void ShopRotationSystem::markShopsRefreshed()
{
    shopsDirty = false;
    rotationIndex++;
}

// EN: getRotationIndex declares or implements a focused behavior used by this module.
// FR: getRotationIndex déclare ou implémente un comportement précis utilisé par ce module.
int ShopRotationSystem::getRotationIndex()
{
    return rotationIndex;
}
