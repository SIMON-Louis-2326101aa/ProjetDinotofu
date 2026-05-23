// EN: WaveRules.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: WaveRules.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_WAVE_WAVERULES_HPP
#define INCLUDE_COMBAT_WAVE_WAVERULES_HPP

class WaveRules
{
public:
    static constexpr int MIN_WAVE_SIZE = 2;
    static constexpr int MAX_WAVE_SIZE = 10;

    // EN: getWaveSizeForPlayerLevel declares or implements a focused behavior used by this module.
    // FR: getWaveSizeForPlayerLevel déclare ou implémente un comportement précis utilisé par ce module.
    static int getWaveSizeForPlayerLevel(int playerLevel);
    // EN: getMonsterLevelForPlayerLevel declares or implements a focused behavior used by this module.
    // FR: getMonsterLevelForPlayerLevel déclare ou implémente un comportement précis utilisé par ce module.
    static int getMonsterLevelForPlayerLevel(int playerLevel, int variation);

private:
    // EN: clamp declares or implements a focused behavior used by this module.
    // FR: clamp déclare ou implémente un comportement précis utilisé par ce module.
    static int clamp(int value, int min, int max);
};

#endif
