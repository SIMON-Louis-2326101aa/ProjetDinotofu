// EN: SummonRules.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: SummonRules.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Provides simple helper rules for future summoning transitions.

#ifndef INCLUDE_COMBAT_SUMMON_SUMMONRULES_HPP
#define INCLUDE_COMBAT_SUMMON_SUMMONRULES_HPP

#include <string>

class SummonRules
{
public:
    // EN: classCanSummon declares or implements a focused behavior used by this module.
    // FR: classCanSummon déclare ou implémente un comportement précis utilisé par ce module.
    static bool classCanSummon(const std::string& className);
    // EN: characterCanSummon declares or implements a focused behavior used by this module.
    // FR: characterCanSummon déclare ou implémente un comportement précis utilisé par ce module.
    static bool characterCanSummon(const std::string& characterName, const std::string& className);
    // EN: getMaxSummonSlots declares or implements a focused behavior used by this module.
    // FR: getMaxSummonSlots déclare ou implémente un comportement précis utilisé par ce module.
    static int getMaxSummonSlots(const std::string& characterName, const std::string& className);
    // EN: getSummonWarningText declares or implements a focused behavior used by this module.
    // FR: getSummonWarningText déclare ou implémente un comportement précis utilisé par ce module.
    static std::string getSummonWarningText(const std::string& className);
};

#endif
