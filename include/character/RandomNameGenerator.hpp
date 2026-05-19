// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Random name generator for normal NPCs and AI fighters.

#ifndef INCLUDE_CHARACTER_RANDOMNAMEGENERATOR_HPP
#define INCLUDE_CHARACTER_RANDOMNAMEGENERATOR_HPP

#include "character/CharacterRace.hpp"
#include "core/Random.hpp"

#include <string>

class RandomNameGenerator
{
public:
    static std::string generateHumanName(Random& random);
    static std::string generateSemiHumanName(Random& random);
    static std::string generateNonHumanName(Random& random);
    static std::string generateNameForRace(CharacterRace race, Random& random);
    static std::string generateAnyName(Random& random);
};

#endif
