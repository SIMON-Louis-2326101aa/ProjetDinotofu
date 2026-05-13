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

void Console::pauseSecondes(int secondes)
{
    std::this_thread::sleep_for(std::chrono::seconds(secondes));
}

void Console::viderBufferEntreeDisponible()
{
    while (std::cin.rdbuf()->in_avail() > 0)
    {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

void Console::attendreEntree()
{
    viderBufferEntreeDisponible();

    std::cout << std::endl;
    std::cout << "Appuie sur Entrée pour continuer...";
    std::cout.flush();

    std::string ligne;
    std::getline(std::cin, ligne);

    viderBufferEntreeDisponible();

    std::cout << std::endl;
}

int Console::demanderNombreEntre(int min, int max, const std::string& messageErreur)
{
    while (true)
    {
        std::string ligne;

        if (!std::getline(std::cin >> std::ws, ligne))
        {
            std::cin.clear();
            std::cout << messageErreur << std::endl;
            std::cout << "> ";
            continue;
        }

        std::istringstream flux(ligne);

        int choix;
        char caractereEnTrop;

        if (!(flux >> choix))
        {
            std::cout << messageErreur << std::endl;
            std::cout << "> ";
            continue;
        }

        if (flux >> caractereEnTrop)
        {
            std::cout << messageErreur << std::endl;
            std::cout << "> ";
            continue;
        }

        if (choix < min || choix > max)
        {
            std::cout << messageErreur << std::endl;
            std::cout << "> ";
            continue;
        }

        viderBufferEntreeDisponible();

        return choix;
    }
}