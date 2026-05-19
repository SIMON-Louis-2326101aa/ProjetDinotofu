// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Prepares race/difficulty-based shop price modifiers.
// Français : Prépare les modificateurs de prix selon la race et la difficulté.

#ifndef INCLUDE_ECONOMY_SHOP_SHOPPRICERULES_HPP
#define INCLUDE_ECONOMY_SHOP_SHOPPRICERULES_HPP

#include <string>

class ShopPriceRules
{
public:
    static int applyBuyModifier(
        int basePrice,
        const std::string& raceName
    );

    static int applySellModifier(
        int basePrice,
        const std::string& raceName
    );

private:
    static bool isDemonLikeRace(const std::string& raceName);
};

#endif
