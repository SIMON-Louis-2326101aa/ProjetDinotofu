// EN: SpecialCharacterGroupCatalog.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterGroupCatalog.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Roadmap catalog for special character relationships and future group encounters.

#ifndef INCLUDE_CHARACTER_RELATIONSHIP_SPECIALCHARACTERGROUPCATALOG_HPP
#define INCLUDE_CHARACTER_RELATIONSHIP_SPECIALCHARACTERGROUPCATALOG_HPP

#include <string>
#include <vector>

class SpecialCharacterGroupCatalog
{
public:
    // EN: displayRoadmap declares or implements a focused behavior used by this module.
    // FR: displayRoadmap déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<std::string> getRoadmapLines();
    static void displayRoadmap();
};

#endif
