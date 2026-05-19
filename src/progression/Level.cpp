// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Implements the first real level curve, still simple enough to tune quickly.
// Français : Implémente la première vraie courbe de niveau, encore simple à équilibrer rapidement.

#include "progression/Level.hpp"

int Level::getExperienceRequiredForNextLevel(int currentLevel)
{
    if (currentLevel < 1)
    {
        currentLevel = 1;
    }

    if (currentLevel >= getMaxLevel())
    {
        return 0;
    }

    return 80 + currentLevel * 35;
}

int Level::getMaxLevel()
{
    return 255;
}
