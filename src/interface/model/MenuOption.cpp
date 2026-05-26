// EN: MenuOption.cpp implements display-neutral menu option data.
// FR: MenuOption.cpp implémente les données neutres d'une option de menu.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/model/MenuOption.hpp"

MenuOption::MenuOption(
    int number,
    const std::string& label,
    const std::string& hint,
    bool enabled,
    const std::string& actionId
)
    : number(number),
      label(label),
      hint(hint),
      actionId(actionId),
      enabled(enabled)
{
}

int MenuOption::getNumber() const
{
    return number;
}

const std::string& MenuOption::getLabel() const
{
    return label;
}

const std::string& MenuOption::getHint() const
{
    return hint;
}

const std::string& MenuOption::getActionId() const
{
    return actionId;
}

bool MenuOption::isEnabled() const
{
    return enabled;
}
