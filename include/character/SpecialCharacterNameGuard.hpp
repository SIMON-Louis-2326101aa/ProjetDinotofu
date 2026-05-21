// EN: SpecialCharacterNameGuard.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterNameGuard.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Guard preventing simple identity theft of special characters during name selection.

#ifndef INCLUDE_CHARACTER_SPECIALCHARACTERNAMEGUARD_HPP
#define INCLUDE_CHARACTER_SPECIALCHARACTERNAMEGUARD_HPP

#include "character/SpecialCharacter.hpp"

#include <string>

class SpecialCharacterNameGuard
{
public:
    // EN: isProtectedName declares or implements a focused behavior used by this module.
    // FR: isProtectedName déclare ou implémente un comportement précis utilisé par ce module.
    static bool isProtectedName(const std::string& name);
    // EN: tryGetProtectedCharacter declares or implements a focused behavior used by this module.
    // FR: tryGetProtectedCharacter déclare ou implémente un comportement précis utilisé par ce module.
    static bool tryGetProtectedCharacter(const std::string& name, SpecialCharacter& result);
    // EN: displayIdentityWarning declares or implements a focused behavior used by this module.
    // FR: displayIdentityWarning déclare ou implémente un comportement précis utilisé par ce module.
    static void displayIdentityWarning(const SpecialCharacter& character);
    // EN: displayIdentityAccepted declares or implements a focused behavior used by this module.
    // FR: displayIdentityAccepted déclare ou implémente un comportement précis utilisé par ce module.
    static void displayIdentityAccepted(const SpecialCharacter& character);
    // EN: displayIdentityRefused declares or implements a focused behavior used by this module.
    // FR: displayIdentityRefused déclare ou implémente un comportement précis utilisé par ce module.
    static void displayIdentityRefused(const SpecialCharacter& character);
};

#endif
