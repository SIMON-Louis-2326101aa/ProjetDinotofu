// EN: ShopRotationSystem.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: ShopRotationSystem.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Prepares shop refreshes after combat sessions.
// Français : Prépare le renouvellement des boutiques après les combats.

#ifndef INCLUDE_ECONOMY_SHOP_SHOPROTATIONSYSTEM_HPP
#define INCLUDE_ECONOMY_SHOP_SHOPROTATIONSYSTEM_HPP

class ShopRotationSystem
{
public:
    // EN: markShopsDirtyAfterCombat declares or implements a focused behavior used by this module.
    // FR: markShopsDirtyAfterCombat déclare ou implémente un comportement précis utilisé par ce module.
    static void markShopsDirtyAfterCombat();
    // EN: shouldRefreshShops declares or implements a focused behavior used by this module.
    // FR: shouldRefreshShops déclare ou implémente un comportement précis utilisé par ce module.
    static bool shouldRefreshShops();
    // EN: markShopsRefreshed declares or implements a focused behavior used by this module.
    // FR: markShopsRefreshed déclare ou implémente un comportement précis utilisé par ce module.
    static void markShopsRefreshed();
    // EN: getRotationIndex declares or implements a focused behavior used by this module.
    // FR: getRotationIndex déclare ou implémente un comportement précis utilisé par ce module.
    static int getRotationIndex();

private:
    static bool shopsDirty;
    static int rotationIndex;
};

#endif
