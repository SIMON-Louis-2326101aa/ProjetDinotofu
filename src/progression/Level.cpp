// EN: Level.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Level.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Implements the first real level curve, still simple enough to tune quickly.
// Français : Implémente la première vraie courbe de niveau, encore simple à équilibrer rapidement.

#include "progression/Level.hpp"

// EN: getExperienceRequiredForNextLevel declares or implements a focused behavior used by this module.
// FR: getExperienceRequiredForNextLevel déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: getMaxLevel declares or implements a focused behavior used by this module.
// FR: getMaxLevel déclare ou implémente un comportement précis utilisé par ce module.
int Level::getMaxLevel()
{
    return 255;
}
