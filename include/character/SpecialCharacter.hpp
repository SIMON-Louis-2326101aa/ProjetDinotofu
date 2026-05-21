// EN: SpecialCharacter.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacter.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Data model for special semi-human characters like Hazak, Aoi, Trexof, Skuro or Sanctus.

#ifndef INCLUDE_CHARACTER_SPECIALCHARACTER_HPP
#define INCLUDE_CHARACTER_SPECIALCHARACTER_HPP

#include "character/CharacterRace.hpp"

#include <string>

class SpecialCharacter
{
private:
    std::string name;
    CharacterRace race;
    std::string nativeClass;
    std::string combatStyle;
    std::string description;
    bool playableWithSpecialDate;
    bool permanentlyNonPlayable;
    std::string specialDate;
    int spawnWeight;

public:
    // EN: SpecialCharacter declares or implements a focused behavior used by this module.
    // FR: SpecialCharacter déclare ou implémente un comportement précis utilisé par ce module.
    SpecialCharacter();

    SpecialCharacter(
        const std::string& name,
        CharacterRace race,
        const std::string& nativeClass,
        const std::string& combatStyle,
        const std::string& description,
        bool playableWithSpecialDate,
        bool permanentlyNonPlayable,
        const std::string& specialDate,
        int spawnWeight
    );

    std::string getName() const;
    // EN: getRace declares or implements a focused behavior used by this module.
    // FR: getRace déclare ou implémente un comportement précis utilisé par ce module.
    CharacterRace getRace() const;
    std::string getRaceText() const;
    std::string getNativeClass() const;
    std::string getCombatStyle() const;
    std::string getDescription() const;
    // EN: canBePlayedWithSpecialDate declares or implements a focused behavior used by this module.
    // FR: canBePlayedWithSpecialDate déclare ou implémente un comportement précis utilisé par ce module.
    bool canBePlayedWithSpecialDate() const;
    // EN: isPermanentlyNonPlayable declares or implements a focused behavior used by this module.
    // FR: isPermanentlyNonPlayable déclare ou implémente un comportement précis utilisé par ce module.
    bool isPermanentlyNonPlayable() const;
    // EN: hasConfiguredSpecialDate declares or implements a focused behavior used by this module.
    // FR: hasConfiguredSpecialDate déclare ou implémente un comportement précis utilisé par ce module.
    bool hasConfiguredSpecialDate() const;
    // EN: matchesSpecialDate declares or implements a focused behavior used by this module.
    // FR: matchesSpecialDate déclare ou implémente un comportement précis utilisé par ce module.
    bool matchesSpecialDate(const std::string& date) const;
    // EN: getSpawnWeight declares or implements a focused behavior used by this module.
    // FR: getSpawnWeight déclare ou implémente un comportement précis utilisé par ce module.
    int getSpawnWeight() const;
};

#endif
