// English: This file prepares random human and semi-human AI opponents for Dinotofu.
// Français : Ce fichier prépare les adversaires IA humains et semi-humains aléatoires de Dinotofu.

#ifndef INCLUDE_CHARACTER_RANDOMCHARACTERGENERATOR_HPP
#define INCLUDE_CHARACTER_RANDOMCHARACTERGENERATOR_HPP

#include "character/CharacterRace.hpp"
#include "class_system/PlayerClass.hpp"
#include "core/Random.hpp"
#include "entity/Player.hpp"

class RandomCharacterGenerator
{
public:
    static constexpr int SPECIAL_ARENA_SPAWN_PERCENTAGE = 25;

    static CharacterRace generateRace(Random& random);

    static Player generateClassicOpponent(Random& random);
    static Player generateClassicOpponentWithClass(
        const PlayerClass& playerClass,
        Random& random
    );

    static Player generateSpecialOpponent(Random& random);
    static Player generateArenaOpponent(Random& random);

    // English: Kept for compatibility with older calls. It never spawns a special character.
    // Français : Conservé pour compatibilité avec les anciens appels. Ne génère jamais de personnage spécial.
    static Player generateRandomOpponent(Random& random);
};

#endif
