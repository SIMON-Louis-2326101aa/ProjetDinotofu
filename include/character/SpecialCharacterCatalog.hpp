// EN: SpecialCharacterCatalog.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterCatalog.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file prepares special semi-human characters for Dinotofu.
// Français : Ce fichier prépare les personnages spéciaux semi-humains de Dinotofu.

#ifndef INCLUDE_CHARACTER_SPECIALCHARACTERCATALOG_HPP
#define INCLUDE_CHARACTER_SPECIALCHARACTERCATALOG_HPP

#include "character/SpecialCharacter.hpp"
#include "core/Random.hpp"

#include <string>
#include <vector>

class SpecialCharacterCatalog
{
public:
    // EN: getAllSpecialCharacters declares or implements a focused behavior used by this module.
    // FR: getAllSpecialCharacters déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<SpecialCharacter> getAllSpecialCharacters();
    // EN: isProtectedName declares or implements a focused behavior used by this module.
    // FR: isProtectedName déclare ou implémente un comportement précis utilisé par ce module.
    static bool isProtectedName(const std::string& name);
    // EN: findByName declares or implements a focused behavior used by this module.
    // FR: findByName déclare ou implémente un comportement précis utilisé par ce module.
    static bool findByName(const std::string& name, SpecialCharacter& result);
    // EN: createRandomSpecialOpponent declares or implements a focused behavior used by this module.
    // FR: createRandomSpecialOpponent déclare ou implémente un comportement précis utilisé par ce module.
    static SpecialCharacter createRandomSpecialOpponent(Random& random);
    // EN: displaySpecialCharactersRoadmap declares or implements a focused behavior used by this module.
    // FR: displaySpecialCharactersRoadmap déclare ou implémente un comportement précis utilisé par ce module.
    static void displaySpecialCharactersRoadmap();

private:
    // EN: normalizeName declares or implements a focused behavior used by this module.
    // FR: normalizeName déclare ou implémente un comportement précis utilisé par ce module.
    static std::string normalizeName(const std::string& name);
};

#endif
