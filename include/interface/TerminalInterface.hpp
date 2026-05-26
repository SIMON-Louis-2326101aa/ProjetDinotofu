// EN: TerminalInterface.hpp renders shared interface models in the terminal.
// FR: TerminalInterface.hpp rend les modèles d'interface partagés dans le terminal.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_TERMINALINTERFACE_HPP
#define INCLUDE_INTERFACE_TERMINALINTERFACE_HPP

#include "interface/model/MenuScreen.hpp"

#include <string>

class TerminalInterface
{
public:
    static void renderMenuScreen(const MenuScreen& screen, bool showPrompt = true);
    static int askMenuChoice(
        const MenuScreen& screen,
        int minChoice,
        int maxChoice,
        const std::string& invalidMessage = "Choix invalide."
    );

    // EN: asks only for one of the real options declared in the screen.
    // FR: demande uniquement une des vraies options déclarées dans l'écran.
    static int askMenuChoiceFromOptions(
        const MenuScreen& screen,
        const std::string& invalidMessage = "Choix invalide.",
        bool allowDisabledOptions = false
    );
};

#endif
