// EN: MenuOption.hpp defines a display-neutral menu option for terminal and future GUI rendering.
// FR: MenuOption.hpp définit une option de menu neutre pour le terminal et la future interface graphique.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MODEL_MENUOPTION_HPP
#define INCLUDE_INTERFACE_MODEL_MENUOPTION_HPP

#include <string>

class MenuOption
{
private:
    int number;
    std::string label;
    std::string hint;
    std::string actionId;
    bool enabled;

public:
    MenuOption(
        int number,
        const std::string& label,
        const std::string& hint = "",
        bool enabled = true,
        const std::string& actionId = ""
    );

    int getNumber() const;
    const std::string& getLabel() const;
    const std::string& getHint() const;
    const std::string& getActionId() const;
    bool isEnabled() const;
};

#endif
