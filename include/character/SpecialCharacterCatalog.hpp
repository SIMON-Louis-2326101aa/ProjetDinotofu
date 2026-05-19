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
    static std::vector<SpecialCharacter> getAllSpecialCharacters();
    static bool isProtectedName(const std::string& name);
    static bool findByName(const std::string& name, SpecialCharacter& result);
    static SpecialCharacter createRandomSpecialOpponent(Random& random);
    static void displaySpecialCharactersRoadmap();

private:
    static std::string normalizeName(const std::string& name);
};

#endif
