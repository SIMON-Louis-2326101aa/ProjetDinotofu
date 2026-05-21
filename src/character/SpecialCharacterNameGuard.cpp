// EN: SpecialCharacterNameGuard.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterNameGuard.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Guard preventing simple identity theft of special characters during name selection.

#include "character/SpecialCharacterNameGuard.hpp"

#include "character/SpecialCharacterCatalog.hpp"

#include <iostream>

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
    std::cout << "Ce nom ne t'appartient pas vraiment." << std::endl;
    std::cout << std::endl;
    std::cout << character.getName()
              << " possède déjà son histoire, ses cicatrices, ses choix et ses fautes."
              << std::endl;
    std::cout << "Usurper son identité n'est pas une simple fantaisie." << std::endl;
    std::cout << std::endl;

    if (character.isPermanentlyNonPlayable())
    {
        std::cout << "Cette identité est verrouillée. Elle n'est pas prévue pour être jouée." << std::endl;
    }
    else
    {
        std::cout << "Si tu es réellement lié à ce personnage, il faudra le prouver." << std::endl;
        std::cout << "La date spéciale peut déverrouiller cette identité." << std::endl;
    }

    std::cout << std::endl;
}

// EN: displayIdentityAccepted declares or implements a focused behavior used by this module.
// FR: displayIdentityAccepted déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterNameGuard::displayIdentityAccepted(const SpecialCharacter& character)
{
    std::cout << "Identité reconnue." << std::endl;
    std::cout << "Tu ne joues pas une copie." << std::endl;
    std::cout << "Tu réveilles une histoire déjà commencée : "
              << character.getName()
              << "."
              << std::endl;
    std::cout << std::endl;
}

// EN: displayIdentityRefused declares or implements a focused behavior used by this module.
// FR: displayIdentityRefused déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterNameGuard::displayIdentityRefused(const SpecialCharacter& character)
{
    std::cout << "Date incorrecte." << std::endl;
    std::cout << character.getName()
              << " refuse de répondre à ton appel."
              << std::endl;
    std::cout << "Choisis un autre nom." << std::endl;
    std::cout << std::endl;
}
