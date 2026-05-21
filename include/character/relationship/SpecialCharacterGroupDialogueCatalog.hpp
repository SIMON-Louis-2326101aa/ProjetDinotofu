// EN: SpecialCharacterGroupDialogueCatalog.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterGroupDialogueCatalog.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Provides relationship dialogue lines for special character groups.

#ifndef INCLUDE_CHARACTER_RELATIONSHIP_SPECIALCHARACTERGROUPDIALOGUECATALOG_HPP
#define INCLUDE_CHARACTER_RELATIONSHIP_SPECIALCHARACTERGROUPDIALOGUECATALOG_HPP

#include <string>
#include <vector>

class SpecialCharacterGroupDialogueCatalog
{
public:
    // EN: displayEntranceDialogue declares or implements a focused behavior used by this module.
    // FR: displayEntranceDialogue déclare ou implémente un comportement précis utilisé par ce module.
    static void displayEntranceDialogue(const std::vector<std::string>& names);
    // EN: displayVictoryDialogue declares or implements a focused behavior used by this module.
    // FR: displayVictoryDialogue déclare ou implémente un comportement précis utilisé par ce module.
    static void displayVictoryDialogue(const std::vector<std::string>& names);
    // EN: displayDefeatDialogue declares or implements a focused behavior used by this module.
    // FR: displayDefeatDialogue déclare ou implémente un comportement précis utilisé par ce module.
    static void displayDefeatDialogue(const std::vector<std::string>& names);

private:
    // EN: containsName declares or implements a focused behavior used by this module.
    // FR: containsName déclare ou implémente un comportement précis utilisé par ce module.
    static bool containsName(const std::vector<std::string>& names, const std::string& expectedName);
    // EN: getEntranceLines declares or implements a focused behavior used by this module.
    // FR: getEntranceLines déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<std::string> getEntranceLines(const std::vector<std::string>& names);
    // EN: getVictoryLines declares or implements a focused behavior used by this module.
    // FR: getVictoryLines déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<std::string> getVictoryLines(const std::vector<std::string>& names);
    // EN: getDefeatLines declares or implements a focused behavior used by this module.
    // FR: getDefeatLines déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<std::string> getDefeatLines(const std::vector<std::string>& names);
    // EN: displayLines declares or implements a focused behavior used by this module.
    // FR: displayLines déclare ou implémente un comportement précis utilisé par ce module.
    static void displayLines(const std::string& title, const std::vector<std::string>& lines);
};

#endif
