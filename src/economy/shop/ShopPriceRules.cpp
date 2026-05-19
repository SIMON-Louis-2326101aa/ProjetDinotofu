// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Implements early shop price modifiers, especially for demon-like races.
// Français : Implémente les premiers modificateurs de prix, notamment pour les races démoniaques.

#include "economy/shop/ShopPriceRules.hpp"

#include <algorithm>
#include <cctype>

int ShopPriceRules::applyBuyModifier(
    int basePrice,
    const std::string& raceName
)
{
    if (basePrice <= 0)
    {
        return 0;
    }

    if (isDemonLikeRace(raceName))
    {
        return basePrice * 115 / 100;
    }

    return basePrice;
}

int ShopPriceRules::applySellModifier(
    int basePrice,
    const std::string& raceName
)
{
    if (basePrice <= 0)
    {
        return 0;
    }

    if (isDemonLikeRace(raceName))
    {
        return basePrice * 85 / 100;
    }

    return basePrice;
}

bool ShopPriceRules::isDemonLikeRace(const std::string& raceName)
{
    std::string loweredRace = raceName;

    std::transform(
        loweredRace.begin(),
        loweredRace.end(),
        loweredRace.begin(),
        [](unsigned char character)
        {
            return static_cast<char>(std::tolower(character));
        }
    );

    return loweredRace.find("démon") != std::string::npos
        || loweredRace.find("demon") != std::string::npos;
}
