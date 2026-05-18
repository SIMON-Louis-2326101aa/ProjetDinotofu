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
    Random();

    int between(int min, int max);
    int rollD20();
    int chooseFirstTurn();
};

#endif
