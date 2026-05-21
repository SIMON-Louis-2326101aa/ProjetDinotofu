// EN: MenuFrame.hpp provides shared helpers for consistent terminal menu rendering.
// FR: MenuFrame.hpp fournit des helpers communs pour afficher des menus terminal cohérents.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_COMMON_MENUFRAME_HPP
#define INCLUDE_INTERFACE_MENU_COMMON_MENUFRAME_HPP

#include <string>

class MenuFrame
{
public:
    // EN: title prints a normalized framed title for terminal menus.
    // FR: title affiche un titre encadré normalisé pour les menus terminal.
    static void title(const std::string& text);

    // EN: subtitle prints a compact contextual line under a menu title.
    // FR: subtitle affiche une ligne de contexte compacte sous un titre de menu.
    static void subtitle(const std::string& text);

    // EN: separator prints a simple horizontal separator.
    // FR: separator affiche une séparation horizontale simple.
    static void separator();

    // EN: option prints a normalized numbered menu option.
    // FR: option affiche une option numérotée de menu de façon normalisée.
    static void option(int number, const std::string& label);

    // EN: backOption prints the normalized return/cancel option.
    // FR: backOption affiche l'option normalisée de retour/annulation.
    static void backOption(const std::string& label = "Retour");

    // EN: prompt prints the terminal input marker.
    // FR: prompt affiche le marqueur de saisie terminal.
    static void prompt();

    // EN: end closes a framed menu visually.
    // FR: end ferme visuellement un menu encadré.
    static void end();
};

#endif
