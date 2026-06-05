// EN: Console.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: Console.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_CORE_CONSOLE_HPP
#define INCLUDE_CORE_CONSOLE_HPP

#include <functional>
#include <string>
#include <vector>

class Console
{
public:
    // EN: configureTerminalEncoding prepares UTF-8 console I/O when supported.
    // FR: configureTerminalEncoding prépare l'encodage UTF-8 du terminal quand c'est possible.
    static void configureTerminalEncoding();

    // EN: printLine writes one UTF-8 terminal line through the central console layer.
    // FR: printLine écrit une ligne terminal UTF-8 via le socle console central.
    static void printLine(const std::string& text);

    // EN: clear declares or implements a focused behavior used by this module.
    // FR: clear déclare ou implémente un comportement précis utilisé par ce module.
    static void clear();
    static void useNormalTheme();
    static void useCombatTheme();
    // EN: pauseSeconds declares or implements a focused behavior used by this module.
    // FR: pauseSeconds déclare ou implémente un comportement précis utilisé par ce module.
    static void pauseSeconds(int seconds);
    // EN: waitForEnter declares or implements a focused behavior used by this module.
    // FR: waitForEnter déclare ou implémente un comportement précis utilisé par ce module.
    static void waitForEnter();

    // EN: askNumberBetween declares or implements a focused behavior used by this module.
    // FR: askNumberBetween déclare ou implémente un comportement précis utilisé par ce module.
    static int askNumberBetween(int min, int max, const std::string& errorMessage);

    // EN: readLine can receive terminal input or a queued GUI input line.
    // FR: readLine peut recevoir une saisie terminal ou une ligne envoyée par l'IG.
    static bool readLine(std::string& line, bool trimLeadingWhitespace = false);

    // EN: Captures legacy terminal output and returns cleaned non-empty lines.
    // FR: Capture les anciens affichages terminal et renvoie les lignes utiles nettoyées.
    static std::vector<std::string> captureLines(const std::function<void()>& action);

    // EN: flushAvailableInputBuffer declares or implements a focused behavior used by this module.
    // FR: flushAvailableInputBuffer déclare ou implémente un comportement précis utilisé par ce module.
    static void flushAvailableInputBuffer();
};

#endif
