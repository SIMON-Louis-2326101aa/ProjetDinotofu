// EN: SpecialCharacter.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacter.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Implementation of the special character data model.

#include "character/SpecialCharacter.hpp"

// EN: SpecialCharacter declares or implements a focused behavior used by this module.
// FR: SpecialCharacter déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: getRace declares or implements a focused behavior used by this module.
// FR: getRace déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: canBePlayedWithSpecialDate declares or implements a focused behavior used by this module.
// FR: canBePlayedWithSpecialDate déclare ou implémente un comportement précis utilisé par ce module.
bool SpecialCharacter::canBePlayedWithSpecialDate() const
{
    return playableWithSpecialDate;
}

// EN: isPermanentlyNonPlayable declares or implements a focused behavior used by this module.
// FR: isPermanentlyNonPlayable déclare ou implémente un comportement précis utilisé par ce module.
bool SpecialCharacter::isPermanentlyNonPlayable() const
{
    return permanentlyNonPlayable;
}

// EN: hasConfiguredSpecialDate declares or implements a focused behavior used by this module.
// FR: hasConfiguredSpecialDate déclare ou implémente un comportement précis utilisé par ce module.
bool SpecialCharacter::hasConfiguredSpecialDate() const
{
    return !specialDate.empty();
}

// EN: matchesSpecialDate declares or implements a focused behavior used by this module.
// FR: matchesSpecialDate déclare ou implémente un comportement précis utilisé par ce module.
bool SpecialCharacter::matchesSpecialDate(const std::string& date) const
{
    return hasConfiguredSpecialDate() && date == specialDate;
}

// EN: getSpawnWeight declares or implements a focused behavior used by this module.
// FR: getSpawnWeight déclare ou implémente un comportement précis utilisé par ce module.
int SpecialCharacter::getSpawnWeight() const
{
    return spawnWeight;
}
