// EN: SpecialEncounterRules.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialEncounterRules.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Provides future rules for special encounter intent and lethal combat permissions.
// TODO: Later, connect these rules to the real combat result, surrender, death and relationship systems.

#ifndef INCLUDE_COMBAT_ENCOUNTER_SPECIALENCOUNTERRULES_HPP
#define INCLUDE_COMBAT_ENCOUNTER_SPECIALENCOUNTERRULES_HPP

#include "combat/encounter/CombatIntent.hpp"

#include <string>
#include <vector>

class SpecialEncounterRules
{
public:
    // EN: getIntentForSpecialGroup declares or implements a focused behavior used by this module.
    // FR: getIntentForSpecialGroup déclare ou implémente un comportement précis utilisé par ce module.
    static CombatIntent getIntentForSpecialGroup(const std::vector<std::string>& names);
    // EN: getIntentText declares or implements a focused behavior used by this module.
    // FR: getIntentText déclare ou implémente un comportement précis utilisé par ce module.
    static std::string getIntentText(CombatIntent intent, const std::vector<std::string>& names);
    // EN: canBecomeDeathMatch declares or implements a focused behavior used by this module.
    // FR: canBecomeDeathMatch déclare ou implémente un comportement précis utilisé par ce module.
    static bool canBecomeDeathMatch(const std::vector<std::string>& names);

private:
    // EN: containsName declares or implements a focused behavior used by this module.
    // FR: containsName déclare ou implémente un comportement précis utilisé par ce module.
    static bool containsName(const std::vector<std::string>& names, const std::string& expectedName);
};

#endif
