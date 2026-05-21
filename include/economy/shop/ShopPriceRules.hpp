// EN: ShopPriceRules.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: ShopPriceRules.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Prepares race/class-based shop price modifiers.
// Français : Prépare les modificateurs de prix selon la race et la classe.

#ifndef INCLUDE_ECONOMY_SHOP_SHOPPRICERULES_HPP
#define INCLUDE_ECONOMY_SHOP_SHOPPRICERULES_HPP

#include <string>

class ShopPriceRules
{
public:
    static int applyBuyModifier(
        int basePrice,
        const std::string& raceName,
        const std::string& className = ""
    );

    static int applySellModifier(
        int basePrice,
        const std::string& raceName,
        const std::string& className = ""
    );

    // EN: hasCraftClassTradeBonus declares or implements a focused behavior used by this module.
    // FR: hasCraftClassTradeBonus déclare ou implémente un comportement précis utilisé par ce module.
    static bool hasCraftClassTradeBonus(const std::string& className);

private:
    // EN: isDemonLikeRace declares or implements a focused behavior used by this module.
    // FR: isDemonLikeRace déclare ou implémente un comportement précis utilisé par ce module.
    static bool isDemonLikeRace(const std::string& raceName);
    // EN: normalizeText declares or implements a focused behavior used by this module.
    // FR: normalizeText déclare ou implémente un comportement précis utilisé par ce module.
    static std::string normalizeText(std::string value);
};

#endif
