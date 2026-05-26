// EN: MessageScreen.cpp implements reusable neutral screens for terminal and future GUI preparation.
// FR: MessageScreen.cpp implémente des écrans neutres réutilisables pour le terminal et la future IG.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/common/MessageScreen.hpp"

#include "core/Console.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"

#include <iostream>

void MessageScreen::show(
    const std::string& title,
    const std::string& screenId,
    const std::vector<std::string>& lines,
    bool waitAndClear
)
{
    MenuScreen screen(title, screenId);

    for (const std::string& line : lines)
    {
        screen.addLine(line);
    }

    TerminalInterface::renderMenuScreen(screen, false);

    if (waitAndClear)
    {
        std::cout << std::endl;
        Console::waitForEnter();
        Console::clear();
    }
}

std::string MessageScreen::askText(
    const std::string& title,
    const std::string& screenId,
    const std::vector<std::string>& lines
)
{
    MenuScreen screen(title, screenId);

    for (const std::string& line : lines)
    {
        screen.addLine(line);
    }

    TerminalInterface::renderMenuScreen(screen, true);

    std::string input;
    if (!std::getline(std::cin >> std::ws, input))
    {
        std::cin.clear();
        return "";
    }

    Console::flushAvailableInputBuffer();
    return input;
}

bool MessageScreen::askKeywordConfirmation(
    const std::string& title,
    const std::string& screenId,
    const std::vector<std::string>& lines,
    const std::string& keyword
)
{
    std::vector<std::string> screenLines = lines;
    screenLines.push_back("");
    screenLines.push_back("Tape " + keyword + " pour confirmer.");

    std::string input = askText(title, screenId, screenLines);
    return input == keyword;
}
