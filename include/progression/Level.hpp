// EN: Level.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: Level.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Centralizes level curve helpers so player progression does not stay hardcoded in Player.
// Français : Centralise les helpers de courbe de niveau pour éviter de garder la progression codée en dur dans Player.

#ifndef INCLUDE_PROGRESSION_LEVEL_HPP
#define INCLUDE_PROGRESSION_LEVEL_HPP

class Level
{
public:
    // EN: getExperienceRequiredForNextLevel declares or implements a focused behavior used by this module.
    // FR: getExperienceRequiredForNextLevel déclare ou implémente un comportement précis utilisé par ce module.
    static int getExperienceRequiredForNextLevel(int currentLevel);
    // EN: getMaxLevel declares or implements a focused behavior used by this module.
    // FR: getMaxLevel déclare ou implémente un comportement précis utilisé par ce module.
    static int getMaxLevel();
};

#endif
