// EN: CombatClassSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatClassSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/system/CombatClassSystem.hpp"

#include <algorithm>
#include <cctype>
#include <string>

std::string CombatClassSystem::normalizeClassText(const std::string& classText)
{
    std::string resultat = classText;

    std::transform(
        resultat.begin(),
        resultat.end(),
        resultat.begin(),
        // EN: [] declares or implements a focused behavior used by this module.
        // FR: [] déclare ou implémente un comportement précis utilisé par ce module.
        [](unsigned char caractere)
        {
            return static_cast<char>(std::tolower(caractere));
        }
    );

    return resultat;
}

// EN: getBaseEscapeChance declares or implements a focused behavior used by this module.
// FR: getBaseEscapeChance déclare ou implémente un comportement précis utilisé par ce module.
int CombatClassSystem::getBaseEscapeChance(const Entity& entity)
{
    std::string className = normalizeClassText(entity.getType());

    if (className.find("assassin") != std::string::npos ||
        className.find("ombrelame") != std::string::npos ||
        className.find("moine") != std::string::npos ||
        className.find("duelliste") != std::string::npos ||
        className.find("légère") != std::string::npos ||
        className.find("legere") != std::string::npos)
    {
        return 72;
    }

    if (className.find("archer") != std::string::npos ||
        className.find("rôdeur") != std::string::npos ||
        className.find("rodeur") != std::string::npos ||
        className.find("chasseur") != std::string::npos ||
        className.find("lanceur de dagues") != std::string::npos ||
        className.find("tireur") != std::string::npos ||
        className.find("artificier") != std::string::npos)
    {
        return 62;
    }

    if (className.find("colosse") != std::string::npos ||
        className.find("gardien") != std::string::npos ||
        className.find("tank") != std::string::npos ||
        className.find("briseur lourd") != std::string::npos ||
        className.find("lourde") != std::string::npos)
    {
        return 28;
    }

    if (className.find("chevalier") != std::string::npos ||
        className.find("paladin") != std::string::npos ||
        className.find("templier") != std::string::npos ||
        className.find("clerc") != std::string::npos ||
        className.find("prêtre") != std::string::npos ||
        className.find("pretre") != std::string::npos)
    {
        return 48;
    }

    return 50;
}

// EN: getBaseDamageReductionPercentage declares or implements a focused behavior used by this module.
// FR: getBaseDamageReductionPercentage déclare ou implémente un comportement précis utilisé par ce module.
int CombatClassSystem::getBaseDamageReductionPercentage(const Entity& entity)
{
    std::string className = normalizeClassText(entity.getType());

    if (className.find("assassin") != std::string::npos ||
        className.find("mage fou") != std::string::npos ||
        className.find("arcaniste") != std::string::npos ||
        className.find("légère") != std::string::npos ||
        className.find("legere") != std::string::npos)
    {
        return 0;
    }

    if (className.find("gardien") != std::string::npos || className.find("tank sac") != std::string::npos)
    {
        return 16;
    }

    if (className.find("colosse") != std::string::npos ||
        className.find("paladin") != std::string::npos ||
        className.find("templier") != std::string::npos ||
        className.find("lourde") != std::string::npos)
    {
        return 11;
    }

    if (className.find("chevalier") != std::string::npos ||
        className.find("guerrier") != std::string::npos ||
        className.find("clerc") != std::string::npos ||
        className.find("prêtre") != std::string::npos ||
        className.find("pretre") != std::string::npos ||
        className.find("druide") != std::string::npos)
    {
        return 6;
    }

    return 0;
}


int CombatClassSystem::getOutgoingDamagePercent(const Entity& entity)
{
    std::string className = normalizeClassText(entity.getType());

    if (className.find("gardien") != std::string::npos
        || className.find("tank sac") != std::string::npos)
    {
        return 82;
    }

    if (className.find("colosse") != std::string::npos
        || className.find("paladin") != std::string::npos)
    {
        return 92;
    }

    if (className.find("assassin") != std::string::npos
        || className.find("ombrelame") != std::string::npos
        || className.find("duelliste") != std::string::npos
        || className.find("lanceur de dagues") != std::string::npos)
    {
        return 112;
    }

    if (className.find("berserker") != std::string::npos
        || className.find("briseur lourd") != std::string::npos
        || className.find("barbare") != std::string::npos)
    {
        return 115;
    }

    if (className.find("duelliste") != std::string::npos
        || className.find("guerrier") != std::string::npos
        || className.find("chevalier runique") != std::string::npos
        || className.find("mage-lame") != std::string::npos)
    {
        return 106;
    }

    if (className.find("pyromancien") != std::string::npos
        || className.find("mage flame") != std::string::npos
        || className.find("mage fou") != std::string::npos)
    {
        return 113;
    }

    if (className.find("mage") != std::string::npos
        || className.find("sorcier") != std::string::npos
        || className.find("ensorceleur") != std::string::npos
        || className.find("arcaniste") != std::string::npos
        || className.find("occultiste") != std::string::npos)
    {
        return 108;
    }

    if (className.find("clerc") != std::string::npos
        || className.find("prêtre") != std::string::npos
        || className.find("pretre") != std::string::npos
        || className.find("barde") != std::string::npos
        || className.find("alchimiste") != std::string::npos)
    {
        return 94;
    }

    return 100;
}

int CombatClassSystem::getOutgoingFlatBonus(const Entity& entity)
{
    std::string className = normalizeClassText(entity.getType());

    if (className.find("lancier") != std::string::npos)
    {
        return 3;
    }

    if (className.find("archer") != std::string::npos
        || className.find("rôdeur") != std::string::npos
        || className.find("rodeur") != std::string::npos
        || className.find("arbal") != std::string::npos
        || className.find("tireur") != std::string::npos)
    {
        return 2;
    }

    if (className.find("forgeron") != std::string::npos)
    {
        return 3;
    }

    if (className.find("arbal") != std::string::npos || className.find("tireur") != std::string::npos)
    {
        return 4;
    }

    if (className.find("moine") != std::string::npos || className.find("duelliste") != std::string::npos)
    {
        return 2;
    }

    return 0;
}
