// EN: SummonRules.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SummonRules.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Provides simple helper rules for future summoning transitions.

#include "combat/summon/SummonRules.hpp"

#include <algorithm>
#include <cctype>

namespace
{
    std::string lower(const std::string& value)
    {
        std::string result = value;
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        return result;
    }
}

// EN: classCanSummon declares or implements a focused behavior used by this module.
// FR: classCanSummon déclare ou implémente un comportement précis utilisé par ce module.
bool SummonRules::classCanSummon(const std::string& className)
{
    return className == "Invocateur"
        || className == "Nécromancien"
        || className == "Dompteur"
        || className == "Pactisant";
}

// EN: characterCanSummon declares or implements a focused behavior used by this module.
// FR: characterCanSummon déclare ou implémente un comportement précis utilisé par ce module.
bool SummonRules::characterCanSummon(const std::string& characterName, const std::string& className)
{
    std::string name = lower(characterName);

    return classCanSummon(className)
        || name == "aoi"
        || name == "hazak"
        || name == "fail"
        || name == "kanadé"
        || name == "kanade";
}

// EN: getMaxSummonSlots declares or implements a focused behavior used by this module.
// FR: getMaxSummonSlots déclare ou implémente un comportement précis utilisé par ce module.
int SummonRules::getMaxSummonSlots(const std::string& characterName, const std::string& className)
{
    std::string name = lower(characterName);

    if (className == "Invocateur") return 3;
    if (className == "Nécromancien") return 2;
    if (className == "Dompteur") return 2;
    if (className == "Pactisant") return 1;
    if (name == "aoi" || name == "hazak" || name == "fail" || name == "kanadé" || name == "kanade") return 1;

    return 0;
}

std::string SummonRules::getSummonWarningText(const std::string& className)
{
    if (!classCanSummon(className))
    {
        return "Cette classe ne possède pas de lien d'invocation assez fort pour ouvrir la meute.";
    }

    return "Cette classe transforme déjà certains combats en mini-groupe grâce aux invocations actives.";
}
