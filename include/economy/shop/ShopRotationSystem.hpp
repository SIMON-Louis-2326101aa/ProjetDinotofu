// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Prepares shop refreshes after combat sessions.
// Français : Prépare le renouvellement des boutiques après les combats.

#ifndef INCLUDE_ECONOMY_SHOP_SHOPROTATIONSYSTEM_HPP
#define INCLUDE_ECONOMY_SHOP_SHOPROTATIONSYSTEM_HPP

class ShopRotationSystem
{
public:
    static void markShopsDirtyAfterCombat();
    static bool shouldRefreshShops();
    static void markShopsRefreshed();

private:
    static bool shopsDirty;
};

#endif
