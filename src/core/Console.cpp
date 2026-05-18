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

void Console::clear()
{
    system("clear");
}

void Console::pauseSeconds(int seconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

void Console::flushAvailableInputBuffer()
{
    while (std::cin.rdbuf()->in_avail() > 0)
    {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

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

int Console::askNumberBetween(int min, int max, const std::string& errorMessage)
{
    while (true)
    {
        std::string ligne;

        if (!std::getline(std::cin >> std::ws, ligne))
        {
            std::cin.clear();
            std::cout << errorMessage << std::endl;
            std::cout << "> ";
            continue;
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