// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Centralizes level curve helpers so player progression does not stay hardcoded in Player.
// Français : Centralise les helpers de courbe de niveau pour éviter de garder la progression codée en dur dans Player.

#ifndef INCLUDE_PROGRESSION_LEVEL_HPP
#define INCLUDE_PROGRESSION_LEVEL_HPP

class Level
{
public:
    static int getExperienceRequiredForNextLevel(int currentLevel);
    static int getMaxLevel();
};

#endif
