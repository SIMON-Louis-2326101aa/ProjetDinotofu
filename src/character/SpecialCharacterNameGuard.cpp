// EN: SpecialCharacterNameGuard.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterNameGuard.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Guard preventing simple identity theft of special characters during name selection.

#include "character/SpecialCharacterNameGuard.hpp"

#include "character/SpecialCharacterCatalog.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <iostream>
#include <vector>

// EN: isProtectedName declares or implements a focused behavior used by this module.
// FR: isProtectedName déclare ou implémente un comportement précis utilisé par ce module.
bool SpecialCharacterNameGuard::isProtectedName(const std::string& name)
{
    return SpecialCharacterCatalog::isProtectedName(name);
}

bool SpecialCharacterNameGuard::tryGetProtectedCharacter(
    const std::string& name,
    SpecialCharacter& result
)
{
    return SpecialCharacterCatalog::findByName(name, result);
}

// EN: displayIdentityWarning declares or implements a focused behavior used by this module.
// FR: displayIdentityWarning déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterNameGuard::displayIdentityWarning(const SpecialCharacter& character)
{
    std::vector<std::string> lines;
    lines.push_back("Ce nom ne t'appartient pas vraiment.");
    lines.push_back(character.getName() + " possède déjà son histoire, ses cicatrices, ses choix et ses fautes.");
    lines.push_back("Usurper son identité n'est pas une simple fantaisie.");

    if (character.isPermanentlyNonPlayable())
    {
        lines.push_back("Cette identité est verrouillée. Elle refuse d'être incarnée par ce chemin.");
    }
    else
    {
        lines.push_back("Si tu es réellement lié à ce personnage, il faudra le prouver.");
        lines.push_back("La date spéciale peut déverrouiller cette identité.");
    }

    MessageScreen::show("IDENTITÉ PROTÉGÉE", "character.special_name.warning", lines);
}

// EN: displayIdentityAccepted declares or implements a focused behavior used by this module.
// FR: displayIdentityAccepted déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterNameGuard::displayIdentityAccepted(const SpecialCharacter& character)
{
    MessageScreen::show(
        "IDENTITÉ RECONNUE",
        "character.special_name.accepted",
        {
            "Identité reconnue.",
            "Tu ne joues pas une copie.",
            "Tu réveilles une histoire déjà commencée : " + character.getName() + "."
        }
    );
}

// EN: displayIdentityRefused declares or implements a focused behavior used by this module.
// FR: displayIdentityRefused déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterNameGuard::displayIdentityRefused(const SpecialCharacter& character)
{
    MessageScreen::show(
        "IDENTITÉ REFUSÉE",
        "character.special_name.refused",
        {
            "Date incorrecte.",
            character.getName() + " refuse de répondre à ton appel.",
            "Choisis un autre nom."
        }
    );
}
