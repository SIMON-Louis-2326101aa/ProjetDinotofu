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
    static CombatIntent getIntentForSpecialGroup(const std::vector<std::string>& names);
    static std::string getIntentText(CombatIntent intent, const std::vector<std::string>& names);
    static bool canBecomeDeathMatch(const std::vector<std::string>& names);

private:
    static bool containsName(const std::vector<std::string>& names, const std::string& expectedName);
};

#endif
