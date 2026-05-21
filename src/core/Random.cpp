// EN: Random.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Random.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "core/Random.hpp"

// EN: Random declares or implements a focused behavior used by this module.
// FR: Random déclare ou implémente un comportement précis utilisé par ce module.
Random::Random() : generator(rd())
{
}

// EN: between declares or implements a focused behavior used by this module.
// FR: between déclare ou implémente un comportement précis utilisé par ce module.
int Random::between(int min, int max)
{
    std::uniform_int_distribution<> distribution(min, max);
    return distribution(generator);
}

// EN: rollD20 declares or implements a focused behavior used by this module.
// FR: rollD20 déclare ou implémente un comportement précis utilisé par ce module.
int Random::rollD20()
{
    return between(1, 20);
}

// EN: chooseFirstTurn declares or implements a focused behavior used by this module.
// FR: chooseFirstTurn déclare ou implémente un comportement précis utilisé par ce module.
int Random::chooseFirstTurn()
{
    return between(1, 2);
}
