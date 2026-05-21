// EN: SpecialCharacterDateRule.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterDateRule.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Implements special-date validation for protected playable identities.

#include "character/SpecialCharacterDateRule.hpp"

#include <cctype>

std::string SpecialCharacterDateRule::normalizeDateInput(const std::string& input)
{
    std::string cleaned;

    for (unsigned char character : input)
    {
        if (!std::isspace(character))
        {
            cleaned += static_cast<char>(character);
        }
    }

    return cleaned;
}

// EN: hasValidDateFormat declares or implements a focused behavior used by this module.
// FR: hasValidDateFormat déclare ou implémente un comportement précis utilisé par ce module.
bool SpecialCharacterDateRule::hasValidDateFormat(const std::string& input)
{
    std::string date = normalizeDateInput(input);

    if (date.size() != 10)
    {
        return false;
    }

    if (date[2] != '/' || date[5] != '/')
    {
        return false;
    }

    for (int i = 0; i < static_cast<int>(date.size()); i++)
    {
        if (i == 2 || i == 5)
        {
            continue;
        }

        if (!std::isdigit(static_cast<unsigned char>(date[i])))
        {
            return false;
        }
    }

    return true;
}

bool SpecialCharacterDateRule::validate(
    const SpecialCharacter& character,
    const std::string& input
)
{
    std::string date = normalizeDateInput(input);

    return hasValidDateFormat(date) && character.matchesSpecialDate(date);
}
