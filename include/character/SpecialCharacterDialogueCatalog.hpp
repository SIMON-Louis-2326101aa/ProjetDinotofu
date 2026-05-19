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
    static bool hasDialogueFor(const std::string& characterName);
    static void displayEntranceDialogue(const std::string& characterName);
    static void displayLowHealthDialogue(const std::string& characterName);
    static void displayVictoryDialogue(const std::string& characterName);
    static void displayDefeatDialogue(const std::string& characterName);

private:
    static std::string normalizeName(const std::string& name);
    static std::vector<std::string> getEntranceLines(const std::string& normalizedName);
    static std::vector<std::string> getLowHealthLines(const std::string& normalizedName);
    static std::vector<std::string> getVictoryLines(const std::string& normalizedName);
    static std::vector<std::string> getDefeatLines(const std::string& normalizedName);
    static void displayLines(const std::string& characterName, const std::vector<std::string>& lines);
};

#endif
