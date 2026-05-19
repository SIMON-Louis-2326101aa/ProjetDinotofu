// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Provides future dialogue lines for special character groups.
// TODO: Later, trigger these dialogues at entrance, low health, victory, defeat and relationship events.

#ifndef INCLUDE_CHARACTER_RELATIONSHIP_SPECIALCHARACTERGROUPDIALOGUECATALOG_HPP
#define INCLUDE_CHARACTER_RELATIONSHIP_SPECIALCHARACTERGROUPDIALOGUECATALOG_HPP

#include <string>
#include <vector>

class SpecialCharacterGroupDialogueCatalog
{
public:
    static void displayEntranceDialogue(const std::vector<std::string>& names);

private:
    static bool containsName(const std::vector<std::string>& names, const std::string& expectedName);
    static std::vector<std::string> getEntranceLines(const std::vector<std::string>& names);
};

#endif
