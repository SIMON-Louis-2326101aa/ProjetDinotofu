// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Provides simple helper rules for future summoning transitions.

#ifndef INCLUDE_COMBAT_SUMMON_SUMMONRULES_HPP
#define INCLUDE_COMBAT_SUMMON_SUMMONRULES_HPP

#include <string>

class SummonRules
{
public:
    static bool classCanSummon(const std::string& className);
    static std::string getSummonWarningText(const std::string& className);
};

#endif
