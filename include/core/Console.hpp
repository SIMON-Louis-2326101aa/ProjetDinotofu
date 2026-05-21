// EN: Console.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: Console.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_CORE_CONSOLE_HPP
#define INCLUDE_CORE_CONSOLE_HPP

#include <string>

class Console
{
public:
    // EN: clear declares or implements a focused behavior used by this module.
    // FR: clear déclare ou implémente un comportement précis utilisé par ce module.
    static void clear();
    // EN: pauseSeconds declares or implements a focused behavior used by this module.
    // FR: pauseSeconds déclare ou implémente un comportement précis utilisé par ce module.
    static void pauseSeconds(int seconds);
    // EN: waitForEnter declares or implements a focused behavior used by this module.
    // FR: waitForEnter déclare ou implémente un comportement précis utilisé par ce module.
    static void waitForEnter();

    // EN: askNumberBetween declares or implements a focused behavior used by this module.
    // FR: askNumberBetween déclare ou implémente un comportement précis utilisé par ce module.
    static int askNumberBetween(int min, int max, const std::string& errorMessage);

    // EN: flushAvailableInputBuffer declares or implements a focused behavior used by this module.
    // FR: flushAvailableInputBuffer déclare ou implémente un comportement précis utilisé par ce module.
    static void flushAvailableInputBuffer();
};

#endif
