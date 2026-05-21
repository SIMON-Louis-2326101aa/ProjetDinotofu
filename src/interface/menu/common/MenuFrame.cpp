// EN: MenuFrame.cpp centralizes small terminal UI helpers used by multiple menus.
// FR: MenuFrame.cpp centralise de petits helpers d'interface terminal utilisés par plusieurs menus.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/common/MenuFrame.hpp"

#include <iostream>

// EN: title prints a normalized framed title for terminal menus.
// FR: title affiche un titre encadré normalisé pour les menus terminal.
void MenuFrame::title(const std::string& text)
{
    std::cout << "========== " << text << " ==========" << std::endl;
}

// EN: subtitle prints a compact contextual line under a menu title.
// FR: subtitle affiche une ligne de contexte compacte sous un titre de menu.
void MenuFrame::subtitle(const std::string& text)
{
    std::cout << text << std::endl;
}

// EN: separator prints a simple horizontal separator.
// FR: separator affiche une séparation horizontale simple.
void MenuFrame::separator()
{
    std::cout << "----------------------------------------" << std::endl;
}

// EN: option prints a normalized numbered menu option.
// FR: option affiche une option numérotée de menu de façon normalisée.
void MenuFrame::option(int number, const std::string& label)
{
    std::cout << number << " : " << label << std::endl;
}

// EN: backOption prints the normalized return/cancel option.
// FR: backOption affiche l'option normalisée de retour/annulation.
void MenuFrame::backOption(const std::string& label)
{
    option(0, label);
}

// EN: prompt prints the terminal input marker.
// FR: prompt affiche le marqueur de saisie terminal.
void MenuFrame::prompt()
{
    std::cout << std::endl;
    std::cout << "> ";
}

// EN: end closes a framed menu visually.
// FR: end ferme visuellement un menu encadré.
void MenuFrame::end()
{
    std::cout << "========================================" << std::endl;
}
