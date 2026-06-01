// EN: main.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: main.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "core/Game.hpp"
#include "core/Console.hpp"
#include "core/VersionInfo.hpp"

#include <string>

// EN: main declares or implements a focused behavior used by this module.
// FR: main déclare ou implémente un comportement précis utilisé par ce module.
int main(int argc, char* argv[])
{
    Console::configureTerminalEncoding();

    if (argc > 1)
    {
        const std::string argument = argv[1];

        if (argument == "--version" || argument == "-v")
        {
            Console::printLine("Dinotofu V" + VersionInfo::currentVersion());
            return 0;
        }

        if (argument == "--encoding-check")
        {
            Console::printLine("Dinotofu V" + VersionInfo::currentVersion());
            Console::printLine("Contrôle encodage terminal : accents français, Épée, ténèbres, forêt, dégâts, Léthal.");
            Console::printLine("Si cette ligne est lisible, le terminal accepte correctement l'UTF-8 pour Dinotofu.");
            return 0;
        }

        if (argument == "--help" || argument == "-h")
        {
            Console::printLine("Dinotofu V" + VersionInfo::currentVersion());
            Console::printLine("Usage : ./Dinotofu [--version] [--encoding-check] [--help]");
            Console::printLine("Sans argument, le jeu se lance normalement.");
            return 0;
        }
    }

    Game jeu;
    jeu.run();

    return 0;
}
