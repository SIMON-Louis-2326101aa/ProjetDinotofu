// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Implements future transformation rules for special characters, especially Sanctus becoming Skuro.

#include "character/SpecialCharacterTransformationRules.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>

namespace
{
    std::string normalize(const std::string& value)
    {
        std::string normalized = value;

        std::transform(
            normalized.begin(),
            normalized.end(),
            normalized.begin(),
            [](unsigned char character)
            {
                return static_cast<char>(std::tolower(character));
            }
        );

        return normalized;
    }
}

bool SpecialCharacterTransformationRules::canTransformIntoSkuro(const std::string& characterName)
{
    return normalize(characterName) == "sanctus";
}

bool SpecialCharacterTransformationRules::shouldSanctusTransformAfterDamageTaken(
    int damageTaken,
    int maxHp
)
{
    if (maxHp <= 0)
    {
        return false;
    }

    return damageTaken * 100 >= maxHp * 35;
}

bool SpecialCharacterTransformationRules::shouldSanctusTransformAfterDamageDealt(
    int damageDealt,
    int targetMaxHp
)
{
    if (targetMaxHp <= 0)
    {
        return false;
    }

    return damageDealt * 100 >= targetMaxHp * 40;
}

void SpecialCharacterTransformationRules::displaySanctusToSkuroForeshadowing()
{
    std::cout << "Quelque chose se fissure derrière la lumière de Sanctus." << std::endl;
    std::cout << "Ce n'est pas une deuxième personne. C'est la même histoire, après la première tête coupée." << std::endl;
    std::cout << "Si la protection se brise trop fort, Skuro pourrait répondre à sa place." << std::endl;
    std::cout << std::endl;
}
