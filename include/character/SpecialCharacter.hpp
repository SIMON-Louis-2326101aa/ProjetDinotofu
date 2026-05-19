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
    CharacterRace getRace() const;
    std::string getRaceText() const;
    std::string getNativeClass() const;
    std::string getCombatStyle() const;
    std::string getDescription() const;
    bool canBePlayedWithSpecialDate() const;
    bool isPermanentlyNonPlayable() const;
    bool hasConfiguredSpecialDate() const;
    bool matchesSpecialDate(const std::string& date) const;
    int getSpawnWeight() const;
};

#endif
