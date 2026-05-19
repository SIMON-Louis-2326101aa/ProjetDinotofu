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
    static bool isProtectedName(const std::string& name);
    static bool tryGetProtectedCharacter(const std::string& name, SpecialCharacter& result);
    static void displayIdentityWarning(const SpecialCharacter& character);
    static void displayIdentityAccepted(const SpecialCharacter& character);
    static void displayIdentityRefused(const SpecialCharacter& character);
};

#endif
