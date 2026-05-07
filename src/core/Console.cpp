#include "core/Console.hpp"

#include <cstdlib>
#include <chrono>
#include <thread>
#include <iostream>
#include <limits>

void Console::clear()
{
    system("clear");
}

void Console::pauseSecondes(int secondes)
{
    std::this_thread::sleep_for(std::chrono::seconds(secondes));
}

void Console::attendreEntree()
{
    std::cout << std::endl;
    std::cout << "Appuie sur Entrée pour continuer...";
    std::cout.flush();

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();

    std::cout << std::endl;
}

int Console::demanderNombreEntre(int min, int max, const std::string& messageErreur)
{
    int choix;

    while (true)
    {
        std::cin >> choix;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << messageErreur << std::endl;
            std::cout << "> ";
            continue;
        }

        if (choix >= min && choix <= max)
        {
            return choix;
        }

        std::cout << messageErreur << std::endl;
        std::cout << "> ";
    }
}