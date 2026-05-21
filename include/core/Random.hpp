// EN: Random.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: Random.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_CORE_RANDOM_HPP
#define INCLUDE_CORE_RANDOM_HPP

#include <random>

class Random
{
private:
    std::random_device rd;
    std::mt19937 generator;

public:
    // EN: Random declares or implements a focused behavior used by this module.
    // FR: Random déclare ou implémente un comportement précis utilisé par ce module.
    Random();

    // EN: between declares or implements a focused behavior used by this module.
    // FR: between déclare ou implémente un comportement précis utilisé par ce module.
    int between(int min, int max);
    // EN: rollD20 declares or implements a focused behavior used by this module.
    // FR: rollD20 déclare ou implémente un comportement précis utilisé par ce module.
    int rollD20();
    // EN: chooseFirstTurn declares or implements a focused behavior used by this module.
    // FR: chooseFirstTurn déclare ou implémente un comportement précis utilisé par ce module.
    int chooseFirstTurn();
};

#endif
