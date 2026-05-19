// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Applies native bonuses for special characters when their intended identity/class is used.

#ifndef INCLUDE_CHARACTER_SPECIALCHARACTERNATIVEBONUS_HPP
#define INCLUDE_CHARACTER_SPECIALCHARACTERNATIVEBONUS_HPP

#include "character/SpecialCharacter.hpp"
#include "entity/Player.hpp"

#include <string>

class SpecialCharacterNativeBonus
{
public:
    static bool applyIfNativeMatch(Player& player);
    static bool applyForSpecialCharacter(Player& player, const SpecialCharacter& character);
    static void displayNativeBonusApplied(const SpecialCharacter& character);

private:
    static bool classMatchesNativeClass(const std::string& playerClass, const std::string& nativeClass);
    static std::string normalize(const std::string& value);
};

#endif
