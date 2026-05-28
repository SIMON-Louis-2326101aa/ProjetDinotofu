// EN: main.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: main.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "core/Game.hpp"
#include "core/Console.hpp"

// EN: main declares or implements a focused behavior used by this module.
// FR: main déclare ou implémente un comportement précis utilisé par ce module.
int main()
{
    Console::configureTerminalEncoding();

    Game jeu;
    jeu.run();

    return 0;
}
