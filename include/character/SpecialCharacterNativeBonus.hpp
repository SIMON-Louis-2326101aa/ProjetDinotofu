// EN: SpecialCharacterNativeBonus.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterNativeBonus.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    // EN: applyIfNativeMatch declares or implements a focused behavior used by this module.
    // FR: applyIfNativeMatch déclare ou implémente un comportement précis utilisé par ce module.
    static bool applyIfNativeMatch(Player& player);
    // EN: applyForSpecialCharacter declares or implements a focused behavior used by this module.
    // FR: applyForSpecialCharacter déclare ou implémente un comportement précis utilisé par ce module.
    static bool applyForSpecialCharacter(Player& player, const SpecialCharacter& character);
    // EN: displayNativeBonusApplied declares or implements a focused behavior used by this module.
    // FR: displayNativeBonusApplied déclare ou implémente un comportement précis utilisé par ce module.
    static void displayNativeBonusApplied(const SpecialCharacter& character);

private:
    // EN: classMatchesNativeClass declares or implements a focused behavior used by this module.
    // FR: classMatchesNativeClass déclare ou implémente un comportement précis utilisé par ce module.
    static bool classMatchesNativeClass(const std::string& playerClass, const std::string& nativeClass);
    // EN: normalize declares or implements a focused behavior used by this module.
    // FR: normalize déclare ou implémente un comportement précis utilisé par ce module.
    static std::string normalize(const std::string& value);
};

#endif
