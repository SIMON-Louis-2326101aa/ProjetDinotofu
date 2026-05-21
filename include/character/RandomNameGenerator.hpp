// EN: RandomNameGenerator.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: RandomNameGenerator.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    // EN: generateHumanName declares or implements a focused behavior used by this module.
    // FR: generateHumanName déclare ou implémente un comportement précis utilisé par ce module.
    static std::string generateHumanName(Random& random);
    // EN: generateSemiHumanName declares or implements a focused behavior used by this module.
    // FR: generateSemiHumanName déclare ou implémente un comportement précis utilisé par ce module.
    static std::string generateSemiHumanName(Random& random);
    // EN: generateNonHumanName declares or implements a focused behavior used by this module.
    // FR: generateNonHumanName déclare ou implémente un comportement précis utilisé par ce module.
    static std::string generateNonHumanName(Random& random);
    // EN: generateNameForRace declares or implements a focused behavior used by this module.
    // FR: generateNameForRace déclare ou implémente un comportement précis utilisé par ce module.
    static std::string generateNameForRace(CharacterRace race, Random& random);
    // EN: generateAnyName declares or implements a focused behavior used by this module.
    // FR: generateAnyName déclare ou implémente un comportement précis utilisé par ce module.
    static std::string generateAnyName(Random& random);
};

#endif
