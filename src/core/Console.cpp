// EN: Console.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Console.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "core/Console.hpp"

#include <cstdlib>
#include <chrono>
#include <thread>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

// EN: clear declares or implements a focused behavior used by this module.
// FR: clear déclare ou implémente un comportement précis utilisé par ce module.
void Console::clear()
{
#if defined(_WIN32)
    system("cls");
#else
    const char* term = std::getenv("TERM");
    if (term != nullptr && std::string(term).empty() == false && std::string(term) != "dumb")
    {
        system("clear");
        return;
    }

    std::cout << std::string(60, '\n');
#endif
}

// EN: pauseSeconds declares or implements a focused behavior used by this module.
// FR: pauseSeconds déclare ou implémente un comportement précis utilisé par ce module.
void Console::pauseSeconds(int seconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

// EN: flushAvailableInputBuffer declares or implements a focused behavior used by this module.
// FR: flushAvailableInputBuffer déclare ou implémente un comportement précis utilisé par ce module.
void Console::flushAvailableInputBuffer()
{
    while (std::cin.rdbuf()->in_avail() > 0)
    {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

// EN: waitForEnter declares or implements a focused behavior used by this module.
// FR: waitForEnter déclare ou implémente un comportement précis utilisé par ce module.
void Console::waitForEnter()
{
    flushAvailableInputBuffer();

    std::cout << std::endl;
    std::cout << "Appuie sur Entrée pour continuer...";
    std::cout.flush();

    std::string ligne;
    std::getline(std::cin, ligne);

    flushAvailableInputBuffer();

    std::cout << std::endl;
}

// EN: askNumberBetween declares or implements a focused behavior used by this module.
// FR: askNumberBetween déclare ou implémente un comportement précis utilisé par ce module.
int Console::askNumberBetween(int min, int max, const std::string& errorMessage)
{
    while (true)
    {
        std::string ligne;

        if (!std::getline(std::cin >> std::ws, ligne))
        {
            std::cin.clear();
            int fallbackChoice = (min <= 0 && 0 <= max) ? 0 : min;
            std::cout << "Entrée interrompue. Choix de secours appliqué." << std::endl;
            return fallbackChoice;
        }

        std::istringstream flux(ligne);

        int choice;
        char caractereEnTrop;

        if (!(flux >> choice))
        {
            std::cout << errorMessage << std::endl;
            std::cout << "> ";
            continue;
        }

        if (flux >> caractereEnTrop)
        {
            std::cout << errorMessage << std::endl;
            std::cout << "> ";
            continue;
        }

        if (choice < min || choice > max)
        {
            std::cout << errorMessage << std::endl;
            std::cout << "> ";
            continue;
        }

        flushAvailableInputBuffer();

        return choice;
    }
}
