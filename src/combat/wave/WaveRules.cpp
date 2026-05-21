// EN: WaveRules.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: WaveRules.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/wave/WaveRules.hpp"

// EN: getWaveSizeForPlayerLevel declares or implements a focused behavior used by this module.
// FR: getWaveSizeForPlayerLevel déclare ou implémente un comportement précis utilisé par ce module.
int WaveRules::getWaveSizeForPlayerLevel(int playerLevel)
{
    int size = MIN_WAVE_SIZE + (playerLevel / 2);

    return clamp(size, MIN_WAVE_SIZE, MAX_WAVE_SIZE);
}

// EN: getMonsterLevelForPlayerLevel declares or implements a focused behavior used by this module.
// FR: getMonsterLevelForPlayerLevel déclare ou implémente un comportement précis utilisé par ce module.
int WaveRules::getMonsterLevelForPlayerLevel(int playerLevel, int variation)
{
    int monsterLevel = playerLevel + variation;

    return clamp(monsterLevel, 1, 99);
}

// EN: clamp declares or implements a focused behavior used by this module.
// FR: clamp déclare ou implémente un comportement précis utilisé par ce module.
int WaveRules::clamp(int value, int min, int max)
{
    if (value < min)
    {
        return min;
    }

    if (value > max)
    {
        return max;
    }

    return value;
}
