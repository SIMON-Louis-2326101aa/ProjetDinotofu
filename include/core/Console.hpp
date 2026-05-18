// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_CORE_CONSOLE_HPP
#define INCLUDE_CORE_CONSOLE_HPP

#include <string>

class Console
{
public:
    static void clear();
    static void pauseSeconds(int seconds);
    static void waitForEnter();

    static int askNumberBetween(int min, int max, const std::string& errorMessage);

    static void flushAvailableInputBuffer();
};

#endif
