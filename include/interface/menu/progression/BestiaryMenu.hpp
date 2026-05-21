// EN: BestiaryMenu.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: BestiaryMenu.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_PROGRESSION_BESTIARYMENU_HPP
#define INCLUDE_INTERFACE_MENU_PROGRESSION_BESTIARYMENU_HPP

#include <string>

class BestiaryMenu
{
public:
    // EN: open declares or implements a focused behavior used by this module.
    // FR: open déclare ou implémente un comportement précis utilisé par ce module.
    static void open();
    // EN: displayObjectEntry declares or implements a focused behavior used by this module.
    // FR: displayObjectEntry déclare ou implémente un comportement précis utilisé par ce module.
    static void displayObjectEntry(const std::string& entryName);
};

#endif
