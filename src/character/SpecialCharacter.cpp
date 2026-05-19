// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Implementation of the special character data model.

#include "character/SpecialCharacter.hpp"

SpecialCharacter::SpecialCharacter()
{
    name = "Inconnu";
    race = CharacterRace::Other;
    nativeClass = "Chevalier";
    combatStyle = "Aucun style défini";
    description = "Personnage spécial non configuré.";
    playableWithSpecialDate = false;
    permanentlyNonPlayable = false;
    specialDate = "";
    spawnWeight = 1;
}

SpecialCharacter::SpecialCharacter(
    const std::string& name,
    CharacterRace race,
    const std::string& nativeClass,
    const std::string& combatStyle,
    const std::string& description,
    bool playableWithSpecialDate,
    bool permanentlyNonPlayable,
    const std::string& specialDate,
    int spawnWeight
)
{
    this->name = name;
    this->race = race;
    this->nativeClass = nativeClass;
    this->combatStyle = combatStyle;
    this->description = description;
    this->playableWithSpecialDate = playableWithSpecialDate;
    this->permanentlyNonPlayable = permanentlyNonPlayable;
    this->specialDate = specialDate;
    this->spawnWeight = spawnWeight;
}

std::string SpecialCharacter::getName() const
{
    return name;
}

CharacterRace SpecialCharacter::getRace() const
{
    return race;
}

std::string SpecialCharacter::getRaceText() const
{
    return characterRaceToText(race);
}

std::string SpecialCharacter::getNativeClass() const
{
    return nativeClass;
}

std::string SpecialCharacter::getCombatStyle() const
{
    return combatStyle;
}

std::string SpecialCharacter::getDescription() const
{
    return description;
}

bool SpecialCharacter::canBePlayedWithSpecialDate() const
{
    return playableWithSpecialDate;
}

bool SpecialCharacter::isPermanentlyNonPlayable() const
{
    return permanentlyNonPlayable;
}

bool SpecialCharacter::hasConfiguredSpecialDate() const
{
    return !specialDate.empty();
}

bool SpecialCharacter::matchesSpecialDate(const std::string& date) const
{
    return hasConfiguredSpecialDate() && date == specialDate;
}

int SpecialCharacter::getSpawnWeight() const
{
    return spawnWeight;
}
