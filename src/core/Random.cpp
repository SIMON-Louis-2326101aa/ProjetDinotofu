// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "core/Random.hpp"

Random::Random() : generator(rd())
{
}

int Random::between(int min, int max)
{
    std::uniform_int_distribution<> distribution(min, max);
    return distribution(generator);
}

int Random::rollD20()
{
    return between(1, 20);
}

int Random::chooseFirstTurn()
{
    return between(1, 2);
}