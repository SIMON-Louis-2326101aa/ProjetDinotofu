// EN: AccountMenu.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: AccountMenu.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Handles local account selection, creation and deletion menus.
// Description : Gère les menus de sélection, création et suppression des comptes locaux.

#ifndef INCLUDE_SAVE_MENU_ACCOUNTMENU_HPP
#define INCLUDE_SAVE_MENU_ACCOUNTMENU_HPP

#include <string>

class AccountMenu
{
public:
    // EN: open declares or implements a focused behavior used by this module.
    // FR: open déclare ou implémente un comportement précis utilisé par ce module.
    static std::string open();
};

#endif
