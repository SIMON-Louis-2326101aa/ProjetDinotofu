// EN: SpecialCharacterDialogueCatalog.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterDialogueCatalog.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Provides personality-based dialogue lines for special characters.
// Description : Fournit des répliques liées à la personnalité des personnages spéciaux.

#ifndef INCLUDE_CHARACTER_SPECIALCHARACTERDIALOGUECATALOG_HPP
#define INCLUDE_CHARACTER_SPECIALCHARACTERDIALOGUECATALOG_HPP

#include <string>
#include <vector>

class SpecialCharacterDialogueCatalog
{
public:
    // EN: hasDialogueFor declares or implements a focused behavior used by this module.
    // FR: hasDialogueFor déclare ou implémente un comportement précis utilisé par ce module.
    static bool hasDialogueFor(const std::string& characterName);
    // EN: displayEntranceDialogue declares or implements a focused behavior used by this module.
    // FR: displayEntranceDialogue déclare ou implémente un comportement précis utilisé par ce module.
    static void displayEntranceDialogue(const std::string& characterName);
    // EN: displayLowHealthDialogue declares or implements a focused behavior used by this module.
    // FR: displayLowHealthDialogue déclare ou implémente un comportement précis utilisé par ce module.
    static void displayLowHealthDialogue(const std::string& characterName);
    // EN: displayCombatActionDialogue declares or implements a focused behavior used by this module.
    // FR: displayCombatActionDialogue déclare ou implémente un comportement précis utilisé par ce module.
    static void displayCombatActionDialogue(const std::string& characterName, const std::string& actionLabel);
    // EN: displayVictoryDialogue declares or implements a focused behavior used by this module.
    // FR: displayVictoryDialogue déclare ou implémente un comportement précis utilisé par ce module.
    static void displayVictoryDialogue(const std::string& characterName);
    // EN: displayDefeatDialogue declares or implements a focused behavior used by this module.
    // FR: displayDefeatDialogue déclare ou implémente un comportement précis utilisé par ce module.
    static void displayDefeatDialogue(const std::string& characterName);

private:
    // EN: normalizeName declares or implements a focused behavior used by this module.
    // FR: normalizeName déclare ou implémente un comportement précis utilisé par ce module.
    static std::string normalizeName(const std::string& name);
    // EN: getEntranceLines declares or implements a focused behavior used by this module.
    // FR: getEntranceLines déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<std::string> getEntranceLines(const std::string& normalizedName);
    // EN: getLowHealthLines declares or implements a focused behavior used by this module.
    // FR: getLowHealthLines déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<std::string> getLowHealthLines(const std::string& normalizedName);
    // EN: getCombatActionLines declares or implements a focused behavior used by this module.
    // FR: getCombatActionLines déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<std::string> getCombatActionLines(const std::string& normalizedName, const std::string& actionLabel);
    // EN: getVictoryLines declares or implements a focused behavior used by this module.
    // FR: getVictoryLines déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<std::string> getVictoryLines(const std::string& normalizedName);
    // EN: getDefeatLines declares or implements a focused behavior used by this module.
    // FR: getDefeatLines déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<std::string> getDefeatLines(const std::string& normalizedName);
    // EN: displayLines declares or implements a focused behavior used by this module.
    // FR: displayLines déclare ou implémente un comportement précis utilisé par ce module.
    static void displayLines(const std::string& characterName, const std::vector<std::string>& lines);
};

#endif
