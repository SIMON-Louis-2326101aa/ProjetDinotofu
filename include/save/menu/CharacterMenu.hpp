// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Handles playable character selection, loading and deletion menus.
// Description : Gère les menus de sélection, chargement et suppression des personnages jouables.

#ifndef INCLUDE_SAVE_MENU_CHARACTERMENU_HPP
#define INCLUDE_SAVE_MENU_CHARACTERMENU_HPP

#include "character/CharacterRace.hpp"
#include "entity/Player.hpp"
#include "progression/DifficultyMode.hpp"

#include <string>

struct CharacterMenuResult
{
    bool characterLoaded;
    bool specialIdentityValidated;
    std::string playerName;
    DifficultyMode difficulty;
    CharacterRace forcedRace;
};

class CharacterMenu
{
public:
    static CharacterMenuResult open(const std::string& accountName, Player& player);
};

#endif
