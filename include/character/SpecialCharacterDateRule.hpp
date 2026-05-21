// EN: SpecialCharacterDateRule.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterDateRule.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Validates special identity dates used to unlock protected playable characters.

#ifndef INCLUDE_CHARACTER_SPECIALCHARACTERDATERULE_HPP
#define INCLUDE_CHARACTER_SPECIALCHARACTERDATERULE_HPP

#include "character/SpecialCharacter.hpp"

#include <string>

class SpecialCharacterDateRule
{
public:
    // EN: normalizeDateInput declares or implements a focused behavior used by this module.
    // FR: normalizeDateInput déclare ou implémente un comportement précis utilisé par ce module.
    static std::string normalizeDateInput(const std::string& input);
    // EN: hasValidDateFormat declares or implements a focused behavior used by this module.
    // FR: hasValidDateFormat déclare ou implémente un comportement précis utilisé par ce module.
    static bool hasValidDateFormat(const std::string& input);
    // EN: validate declares or implements a focused behavior used by this module.
    // FR: validate déclare ou implémente un comportement précis utilisé par ce module.
    static bool validate(const SpecialCharacter& character, const std::string& input);
};

#endif
