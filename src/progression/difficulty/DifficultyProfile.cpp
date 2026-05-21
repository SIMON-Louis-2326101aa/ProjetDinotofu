// EN: DifficultyProfile.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: DifficultyProfile.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Centralizes difficulty-specific percentages for combat, escape, death and rewards.

#include "progression/difficulty/DifficultyProfile.hpp"

// EN: forMode declares or implements a focused behavior used by this module.
// FR: forMode déclare ou implémente un comportement précis utilisé par ce module.
DifficultyProfile DifficultyProfile::forMode(DifficultyMode difficulty)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return {
                15, 90, 90, -6,
                90, 35, 90, 110,
                75, 35,
                10, 10, 5, 75,
                75, 1, 0, 0, 0,
                35, 10, 5, 2, 1
            };

        case DifficultyMode::Hard:
            return {
                -8, 112, 115, 6,
                35, 20, 115, 90,
                55, 15,
                20, 20, 15, 30,
                30, -1, -1, 20, 25,
                55, 25, 12, 5, 3
            };

        case DifficultyMode::Nightmare:
            return {
                -15, 125, 130, 11,
                25, 10, 130, 75,
                45, 8,
                25, 25, 20, 10,
                20, -2, -1, 30, 35,
                65, 30, 18, 7, 5
            };

        case DifficultyMode::Lethal:
            return {
                -18, 135, 140, 14,
                25, 10, 150, 65,
                40, 0,
                0, 0, 0, 1,
                15, -2, -1, 35, 45,
                100, 40, 25, 9, 7
            };

        case DifficultyMode::Normal:
        default:
            return {
                0, 100, 100, 0,
                50, 25, 100, 100,
                65, 25,
                15, 15, 10, 50,
                50, 0, 0, 0, 0,
                50, 20, 10, 4, 2
            };
    }
}
