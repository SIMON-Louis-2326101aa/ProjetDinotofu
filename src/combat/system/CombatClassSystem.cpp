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
        [](unsigned char caractere)
        {
            return static_cast<char>(std::tolower(caractere));
        }
    );

    return resultat;
}

int CombatClassSystem::getBaseEscapeChance(const Entity& entity)
{
    std::string className = normalizeClassText(entity.getType());

    if (className.find("assassin") != std::string::npos ||
        className.find("légère") != std::string::npos ||
        className.find("legere") != std::string::npos)
    {
        return 70;
    }

    if (className.find("colosse") != std::string::npos ||
        className.find("lourde") != std::string::npos)
    {
        return 30;
    }

    if (className.find("chevalier") != std::string::npos ||
        className.find("paladin") != std::string::npos)
    {
        return 50;
    }

    return 50;
}

int CombatClassSystem::getBaseDamageReductionPercentage(const Entity& entity)
{
    std::string className = normalizeClassText(entity.getType());

    if (className.find("assassin") != std::string::npos ||
        className.find("légère") != std::string::npos ||
        className.find("legere") != std::string::npos)
    {
        return 0;
    }

    if (className.find("colosse") != std::string::npos ||
        className.find("lourde") != std::string::npos)
    {
        return 10;
    }

    if (className.find("chevalier") != std::string::npos ||
        className.find("paladin") != std::string::npos)
    {
        return 5;
    }

    return 0;
}