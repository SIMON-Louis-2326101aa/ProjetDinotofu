// EN: TerminalInterface.cpp renders shared interface models in the current terminal UI.
// FR: TerminalInterface.cpp rend les modèles d'interface partagés dans l'interface terminal actuelle.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/TerminalInterface.hpp"

#include "core/Console.hpp"
#include "interface/menu/common/MenuFrame.hpp"
#include "interface/GuiDebugExporter.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void TerminalInterface::renderMenuScreen(const MenuScreen& screen, bool showPrompt)
{
    GuiDebugExporter::exportMenu(screen);

    MenuFrame::title(screen.getTitle());

    for (const std::string& subtitle : screen.getSubtitles())
    {
        MenuFrame::subtitle(subtitle);
    }

    if (!screen.getSubtitles().empty() && (!screen.getLines().empty() || !screen.getOptions().empty()))
    {
        MenuFrame::separator();
    }

    for (const std::string& line : screen.getLines())
    {
        std::cout << line << std::endl;
    }

    if (!screen.getLines().empty() && !screen.getOptions().empty())
    {
        MenuFrame::separator();
    }

    for (const MenuOption& option : screen.getOptions())
    {
        std::string label = option.getLabel();

        if (!option.isEnabled())
        {
            label = "[◘ " + label + "]";
        }

        MenuFrame::option(option.getNumber(), label);

        if (!option.getHint().empty())
        {
            std::cout << "    " << option.getHint() << std::endl;
        }
    }

    if (!screen.getFooterLines().empty())
    {
        MenuFrame::separator();
        for (const std::string& line : screen.getFooterLines())
        {
            std::cout << line << std::endl;
        }
    }

    MenuFrame::end();

    if (showPrompt)
    {
        MenuFrame::prompt();
    }
}

int TerminalInterface::askMenuChoice(
    const MenuScreen& screen,
    int minChoice,
    int maxChoice,
    const std::string& invalidMessage
)
{
    renderMenuScreen(screen);
    return Console::askNumberBetween(minChoice, maxChoice, invalidMessage);
}


int TerminalInterface::askMenuChoiceFromOptions(
    const MenuScreen& screen,
    const std::string& invalidMessage,
    bool allowDisabledOptions
)
{
    renderMenuScreen(screen);

    std::vector<int> allowedChoices;

    for (const MenuOption& option : screen.getOptions())
    {
        if (allowDisabledOptions || option.isEnabled())
        {
            allowedChoices.push_back(option.getNumber());
        }
    }

    while (true)
    {
        std::string line;

        if (!Console::readLine(line, true))
        {
            std::cin.clear();
            std::cout << "Entrée interrompue. Retour appliqué." << std::endl;
            return 0;
        }

        std::istringstream stream(line);
        int choice = 0;
        char extraCharacter = '\0';

        if (!(stream >> choice) || (stream >> extraCharacter))
        {
            std::cout << invalidMessage << std::endl;
            MenuFrame::prompt();
            continue;
        }

        if (std::find(allowedChoices.begin(), allowedChoices.end(), choice) == allowedChoices.end())
        {
            std::cout << invalidMessage << std::endl;
            MenuFrame::prompt();
            continue;
        }

        Console::flushAvailableInputBuffer();
        return choice;
    }
}
