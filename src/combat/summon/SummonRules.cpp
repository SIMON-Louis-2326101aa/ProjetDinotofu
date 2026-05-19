// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Provides simple helper rules for future summoning transitions.

#include "combat/summon/SummonRules.hpp"

bool SummonRules::classCanSummon(const std::string& className)
{
    return className == "Invocateur"
        || className == "Nécromancien"
        || className == "Dompteur"
        || className == "Pactisant";
}

std::string SummonRules::getSummonWarningText(const std::string& className)
{
    if (!classCanSummon(className))
    {
        return "Cette classe ne transforme pas encore le duel en combat d'invocations.";
    }

    return "Cette classe pourra transformer un duel en mini-PvE dès que les invocations actives seront branchées.";
}
