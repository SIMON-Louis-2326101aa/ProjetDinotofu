// EN: MenuScreen.cpp implements a display-neutral screen model used during GUI preparation.
// FR: MenuScreen.cpp implémente un modèle d'écran neutre utilisé pour préparer l'IG.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/model/MenuScreen.hpp"

#include <algorithm>

MenuScreen::MenuScreen(const std::string& title, const std::string& screenId)
    : screenId(screenId),
      title(title)
{
}

void MenuScreen::setTitle(const std::string& value)
{
    title = value;
}

void MenuScreen::setScreenId(const std::string& value)
{
    screenId = value;
}

void MenuScreen::addSubtitle(const std::string& value)
{
    subtitles.push_back(value);
}

void MenuScreen::addLine(const std::string& value)
{
    lines.push_back(value);
}

void MenuScreen::addFooterLine(const std::string& value)
{
    footerLines.push_back(value);
}

void MenuScreen::addOption(
    int number,
    const std::string& label,
    const std::string& hint,
    bool enabled,
    const std::string& actionId
)
{
    options.emplace_back(number, label, hint, enabled, actionId);
}

void MenuScreen::addBackOption(const std::string& label, const std::string& actionId)
{
    addOption(0, label, "", true, actionId);
}

const std::string& MenuScreen::getScreenId() const
{
    return screenId;
}

const std::string& MenuScreen::getTitle() const
{
    return title;
}

const std::vector<std::string>& MenuScreen::getSubtitles() const
{
    return subtitles;
}

const std::vector<std::string>& MenuScreen::getLines() const
{
    return lines;
}

const std::vector<MenuOption>& MenuScreen::getOptions() const
{
    return options;
}

const std::vector<std::string>& MenuScreen::getFooterLines() const
{
    return footerLines;
}

int MenuScreen::getHighestOptionNumber() const
{
    int highest = 0;

    for (const MenuOption& option : options)
    {
        highest = std::max(highest, option.getNumber());
    }

    return highest;
}


GuiMenuSnapshot MenuScreen::toGuiSnapshot() const
{
    GuiMenuSnapshot snapshot;
    snapshot.screenId = screenId;
    snapshot.title = title;
    snapshot.subtitles = subtitles;
    snapshot.lines = lines;
    snapshot.footerLines = footerLines;

    for (const MenuOption& option : options)
    {
        GuiMenuActionSnapshot action;
        action.number = option.getNumber();
        action.label = option.getLabel();
        action.hint = option.getHint();
        action.enabled = option.isEnabled();
        action.actionId = option.getActionId();
        snapshot.actions.push_back(action);
    }

    return snapshot;
}
