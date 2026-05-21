// EN: RandomCharacterGenerator.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: RandomCharacterGenerator.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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

    // EN: generateRace declares or implements a focused behavior used by this module.
    // FR: generateRace déclare ou implémente un comportement précis utilisé par ce module.
    static CharacterRace generateRace(Random& random);

    // EN: generateClassicOpponent declares or implements a focused behavior used by this module.
    // FR: generateClassicOpponent déclare ou implémente un comportement précis utilisé par ce module.
    static Player generateClassicOpponent(Random& random);
    static Player generateClassicOpponentWithClass(
        const PlayerClass& playerClass,
        Random& random
    );

    // EN: generateSpecialOpponent declares or implements a focused behavior used by this module.
    // FR: generateSpecialOpponent déclare ou implémente un comportement précis utilisé par ce module.
    static Player generateSpecialOpponent(Random& random);
    // EN: generateArenaOpponent declares or implements a focused behavior used by this module.
    // FR: generateArenaOpponent déclare ou implémente un comportement précis utilisé par ce module.
    static Player generateArenaOpponent(Random& random);

    // English: Kept for compatibility with older calls. It never spawns a special character.
    // Français : Conservé pour compatibilité avec les anciens appels. Ne génère jamais de personnage spécial.
    // EN: generateRandomOpponent declares or implements a focused behavior used by this module.
    // FR: generateRandomOpponent déclare ou implémente un comportement précis utilisé par ce module.
    static Player generateRandomOpponent(Random& random);
};

#endif
