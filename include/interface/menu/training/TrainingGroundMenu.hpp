// EN: TrainingGroundMenu centralizes paid training sessions between combats.
// FR: TrainingGroundMenu centralise les séances d'entraînement louables entre deux combats.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_TRAINING_TRAININGGROUNDMENU_HPP
#define INCLUDE_INTERFACE_MENU_TRAINING_TRAININGGROUNDMENU_HPP

class Player;

class TrainingGroundMenu
{
public:
    static bool open(Player& player);
};

#endif
