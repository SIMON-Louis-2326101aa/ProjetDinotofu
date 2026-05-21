// EN: ShopPriceRules.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: ShopPriceRules.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Implements early shop price modifiers, especially race and craft-class trading.
// Français : Implémente les premiers modificateurs de prix, notamment selon la race et les classes d'artisanat.

#include "economy/shop/ShopPriceRules.hpp"

#include <algorithm>
#include <cctype>

int ShopPriceRules::applyBuyModifier(
    int basePrice,
    const std::string& raceName,
    const std::string& className
)
{
    if (basePrice <= 0)
    {
        return 0;
    }

    int modifiedPrice = basePrice;

    if (isDemonLikeRace(raceName))
    {
        modifiedPrice = modifiedPrice * 115 / 100;
    }

    std::string loweredClass = normalizeText(className);

    if (loweredClass == "alchimiste")
    {
        modifiedPrice = modifiedPrice * 94 / 100;
    }
    else if (loweredClass == "forgeron")
    {
        modifiedPrice = modifiedPrice * 95 / 100;
    }
    else if (loweredClass == "artificier")
    {
        modifiedPrice = modifiedPrice * 96 / 100;
    }

    return std::max(1, modifiedPrice);
}

int ShopPriceRules::applySellModifier(
    int basePrice,
    const std::string& raceName,
    const std::string& className
)
{
    if (basePrice <= 0)
    {
        return 0;
    }

    int modifiedPrice = basePrice;

    if (isDemonLikeRace(raceName))
    {
        modifiedPrice = modifiedPrice * 85 / 100;
    }

    std::string loweredClass = normalizeText(className);

    if (loweredClass == "forgeron")
    {
        modifiedPrice = modifiedPrice * 110 / 100;
    }
    else if (loweredClass == "alchimiste")
    {
        modifiedPrice = modifiedPrice * 109 / 100;
    }
    else if (loweredClass == "artificier")
    {
        modifiedPrice = modifiedPrice * 112 / 100;
    }

    return std::max(1, modifiedPrice);
}

// EN: hasCraftClassTradeBonus declares or implements a focused behavior used by this module.
// FR: hasCraftClassTradeBonus déclare ou implémente un comportement précis utilisé par ce module.
bool ShopPriceRules::hasCraftClassTradeBonus(const std::string& className)
{
    std::string loweredClass = normalizeText(className);

    return loweredClass == "forgeron"
        || loweredClass == "alchimiste"
        || loweredClass == "artificier";
}

// EN: isDemonLikeRace declares or implements a focused behavior used by this module.
// FR: isDemonLikeRace déclare ou implémente un comportement précis utilisé par ce module.
bool ShopPriceRules::isDemonLikeRace(const std::string& raceName)
{
    std::string loweredRace = normalizeText(raceName);

    return loweredRace.find("démon") != std::string::npos
        || loweredRace.find("demon") != std::string::npos;
}

std::string ShopPriceRules::normalizeText(std::string value)
{
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        // EN: [] declares or implements a focused behavior used by this module.
        // FR: [] déclare ou implémente un comportement précis utilisé par ce module.
        [](unsigned char character)
        {
            return static_cast<char>(std::tolower(character));
        }
    );

    return value;
}
