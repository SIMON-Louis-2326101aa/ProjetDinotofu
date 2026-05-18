// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_WAVE_WAVERULES_HPP
#define INCLUDE_COMBAT_WAVE_WAVERULES_HPP

class WaveRules
{
public:
    static constexpr int MIN_WAVE_SIZE = 3;
    static constexpr int MAX_WAVE_SIZE = 10;

    static int getWaveSizeForPlayerLevel(int playerLevel);
    static int getMonsterLevelForPlayerLevel(int playerLevel, int variation);

private:
    static int clamp(int value, int min, int max);
};

#endif
