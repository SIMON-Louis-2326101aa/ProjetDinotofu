// EN: MessageScreen.hpp centralizes simple information, input and keyword confirmation screens.
// FR: MessageScreen.hpp centralise les écrans simples d'information, de saisie et de confirmation par mot-clé.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_COMMON_MESSAGESCREEN_HPP
#define INCLUDE_INTERFACE_MENU_COMMON_MESSAGESCREEN_HPP

#include <string>
#include <vector>

class MessageScreen
{
public:
    // EN: show displays a simple reusable message screen.
    // FR: show affiche un écran de message simple et réutilisable.
    static void show(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        bool waitAndClear = true
    );

    // EN: askText displays a reusable input screen and returns the entered text.
    // FR: askText affiche un écran de saisie réutilisable et renvoie le texte entré.
    static std::string askText(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        const std::string& placeholder = "Saisis ta réponse ici",
        const std::string& hint = "Texte libre attendu.",
        bool allowEmpty = false,
        int minLength = 0,
        int maxLength = 120
    );

    // EN: askQuantity displays a structured quantity screen for terminal and GUI input.
    // FR: askQuantity affiche un écran de quantité structuré pour le terminal et l'IG.
    static int askQuantity(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        int minValue,
        int maxValue,
        const std::string& invalidMessage = "Quantité invalide."
    );

    // EN: askKeywordConfirmation asks for an exact keyword before a dangerous action.
    // FR: askKeywordConfirmation demande un mot-clé exact avant une action dangereuse.
    static bool askKeywordConfirmation(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        const std::string& keyword
    );
};

#endif
