// EN: CombatMenu.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatMenu.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_COMBATMENU_HPP
#define INCLUDE_INTERFACE_MENU_COMBATMENU_HPP

#include "entity/Entity.hpp"
#include "interface/model/MenuScreen.hpp"

class CombatMenu
{
public:
    static MenuScreen buildTurnScreen(const Entity& entity);

    // EN: displayTurnMenu renders the current terminal turn menu from the shared screen model.
    // FR: displayTurnMenu affiche le menu de tour terminal depuis le modèle partagé.
    static void displayTurnMenu(const Entity& entity);
    // EN: displayUnavailableOption declares or implements a focused behavior used by this module.
    // FR: displayUnavailableOption déclare ou implémente un comportement précis utilisé par ce module.
    static void displayUnavailableOption();
};

#endif
