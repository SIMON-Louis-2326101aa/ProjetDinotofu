// EN: PostCombatMenu.hpp centralizes the post-combat menu as shared screen data.
// FR: PostCombatMenu.hpp centralise le menu d'après-combat sous forme de données d'écran partagées.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_POSTCOMBATMENU_HPP
#define INCLUDE_INTERFACE_MENU_POSTCOMBATMENU_HPP

#include "entity/Player.hpp"
#include "interface/model/MenuScreen.hpp"

class PostCombatMenu
{
public:
    static MenuScreen buildScreen(const Player& player);
    static void display(const Player& player);
    static int getMaxChoice(const Player& player);
};

#endif
