// EN: MessageScreen.cpp implements reusable neutral screens for terminal and future GUI preparation.
// FR: MessageScreen.cpp implémente des écrans neutres réutilisables pour le terminal et la future IG.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/common/MessageScreen.hpp"

#include "core/Console.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"

#include <iostream>
#include <string>

void MessageScreen::show(
    const std::string& title,
    const std::string& screenId,
    const std::vector<std::string>& lines,
    bool waitAndClear
)
{
    MenuScreen screen(title, screenId);
    if (waitAndClear)
    {
        screen.setContinueInput("Valide pour continuer.");
    }
    else
    {
        screen.setDisplayOnlyInput("Information affichée sans saisie obligatoire.");
    }

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
    const std::vector<std::string>& lines,
    const std::string& placeholder,
    const std::string& hint,
    bool allowEmpty,
    int minLength,
    int maxLength
)
{
    const int normalizedMinLength = minLength < 0 ? 0 : minLength;
    const int normalizedMaxLength = maxLength < 0 ? 0 : maxLength;

    while (true)
    {
        MenuScreen screen(title, screenId);
        screen.setTextInput(placeholder, hint, allowEmpty, normalizedMinLength, normalizedMaxLength);

        for (const std::string& line : lines)
        {
            screen.addLine(line);
        }

        TerminalInterface::renderMenuScreen(screen, true);

        std::string input;
        if (!Console::readLine(input, true))
        {
            std::cin.clear();
            return "";
        }

        Console::flushAvailableInputBuffer();

        const int inputLength = static_cast<int>(input.size());
        if (input.empty() && allowEmpty)
        {
            return input;
        }

        if (!allowEmpty && input.empty())
        {
            Console::clear();
            MessageScreen::show(
                "SAISIE REFUSÉE",
                screenId + ".empty",
                {
                    "Les archives refusent une réponse vide.",
                    "Entre une vraie valeur pour continuer."
                }
            );
            continue;
        }

        if (normalizedMinLength > 0 && inputLength < normalizedMinLength)
        {
            Console::clear();
            MessageScreen::show(
                "SAISIE TROP COURTE",
                screenId + ".too_short",
                {
                    "Texte trop court : minimum " + std::to_string(normalizedMinLength) + " caractère(s).",
                    "Les archives refusent de graver une réponse aussi courte."
                }
            );
            continue;
        }

        if (normalizedMaxLength > 0 && inputLength > normalizedMaxLength)
        {
            Console::clear();
            MessageScreen::show(
                "SAISIE TROP LONGUE",
                screenId + ".too_long",
                {
                    "Texte trop long : maximum " + std::to_string(normalizedMaxLength) + " caractère(s).",
                    "Même les parchemins aiment quand ça tient dans la marge."
                }
            );
            continue;
        }

        return input;
    }
}

int MessageScreen::askQuantity(
    const std::string& title,
    const std::string& screenId,
    const std::vector<std::string>& lines,
    int minValue,
    int maxValue,
    const std::string& invalidMessage
)
{
    MenuScreen screen(title, screenId);
    screen.setQuantityInput(minValue, maxValue);

    for (const std::string& line : lines)
    {
        screen.addLine(line);
    }

    TerminalInterface::renderMenuScreen(screen, true);
    return Console::askNumberBetween(minValue, maxValue, invalidMessage);
}

bool MessageScreen::askKeywordConfirmation(
    const std::string& title,
    const std::string& screenId,
    const std::vector<std::string>& lines,
    const std::string& keyword
)
{
    MenuScreen screen(title, screenId);
    screen.setConfirmationInput(keyword, "Confirmation exacte requise avant de continuer.");

    for (const std::string& line : lines)
    {
        screen.addLine(line);
    }

    screen.addLine("");
    screen.addLine("Tape " + keyword + " pour confirmer.");

    TerminalInterface::renderMenuScreen(screen, true);

    std::string input;
    if (!Console::readLine(input, true))
    {
        std::cin.clear();
        return false;
    }

    Console::flushAvailableInputBuffer();
    return input == keyword;
}
