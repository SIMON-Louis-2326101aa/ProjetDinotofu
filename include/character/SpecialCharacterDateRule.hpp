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
    static std::string normalizeDateInput(const std::string& input);
    static bool hasValidDateFormat(const std::string& input);
    static bool validate(const SpecialCharacter& character, const std::string& input);
};

#endif
