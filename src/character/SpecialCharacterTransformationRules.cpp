// EN: SpecialCharacterTransformationRules.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterTransformationRules.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Implements future transformation rules for special characters, especially Sanctus becoming Skuro.

#include "character/SpecialCharacterTransformationRules.hpp"

#include <algorithm>
#include <cctype>
#include "interface/menu/common/MessageScreen.hpp"

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
            // EN: [] declares or implements a focused behavior used by this module.
            // FR: [] déclare ou implémente un comportement précis utilisé par ce module.
            [](unsigned char character)
            {
                return static_cast<char>(std::tolower(character));
            }
        );

        return normalized;
    }
}

// EN: canTransformIntoSkuro declares or implements a focused behavior used by this module.
// FR: canTransformIntoSkuro déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: displaySanctusToSkuroForeshadowing declares or implements a focused behavior used by this module.
// FR: displaySanctusToSkuroForeshadowing déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterTransformationRules::displaySanctusToSkuroForeshadowing()
{
    MessageScreen::show(
        "PRÉSAGE",
        "character.transformation.sanctus_skuro",
        {
            "Quelque chose se fissure derrière la lumière de Sanctus.",
            "Ce n'est pas une deuxième personne. C'est la même histoire, après la première tête coupée.",
            "Si la protection se brise trop fort, Skuro pourrait répondre à sa place."
        },
        false
    );
}
