// EN: DeathRuleRules.cpp centralizes character death-rule forcing and labels.
// FR: DeathRuleRules.cpp centralise les règles forcées et les libellés de mort.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "progression/DeathRuleRules.hpp"

#include <algorithm>
#include <cctype>

namespace
{
    std::string normalizeDeathRuleText(const std::string& value)
    {
        std::string normalized;

        for (unsigned char character : value)
        {
            if (std::isalnum(character))
            {
                normalized += static_cast<char>(std::tolower(character));
            }
        }

        return normalized;
    }
}

DeathRuleMode DeathRuleRules::normalizeForDifficulty(DifficultyMode difficulty, DeathRuleMode requestedRule)
{
    if (difficulty == DifficultyMode::Easy)
    {
        return DeathRuleMode::NonDefinitive;
    }

    if (difficulty == DifficultyMode::Lethal)
    {
        return DeathRuleMode::Definitive;
    }

    return requestedRule;
}

DeathRuleMode DeathRuleRules::defaultForDifficulty(DifficultyMode difficulty)
{
    return normalizeForDifficulty(difficulty, DeathRuleMode::NonDefinitive);
}

bool DeathRuleRules::isChoiceForced(DifficultyMode difficulty)
{
    return difficulty == DifficultyMode::Easy || difficulty == DifficultyMode::Lethal;
}

bool DeathRuleRules::isDefinitive(DifficultyMode difficulty, DeathRuleMode deathRule)
{
    return normalizeForDifficulty(difficulty, deathRule) == DeathRuleMode::Definitive;
}

std::string DeathRuleRules::toSaveText(DeathRuleMode deathRule)
{
    switch (deathRule)
    {
        case DeathRuleMode::Definitive:
            return "Définitive";
        case DeathRuleMode::NonDefinitive:
        default:
            return "Non définitive";
    }
}

DeathRuleMode DeathRuleRules::fromSaveText(const std::string& value, DifficultyMode difficulty)
{
    const std::string normalized = normalizeDeathRuleText(value);

    if (normalized == "definitive" || normalized == "dfinitive" || normalized == "permadeath" || normalized == "permanent" || normalized == "lethal" || normalized == "lthal")
    {
        return normalizeForDifficulty(difficulty, DeathRuleMode::Definitive);
    }

    if (normalized == "nondefinitive" || normalized == "nondef" || normalized == "safe" || normalized == "standard" || normalized == "normal")
    {
        return normalizeForDifficulty(difficulty, DeathRuleMode::NonDefinitive);
    }

    return defaultForDifficulty(difficulty);
}

std::string DeathRuleRules::displayName(DeathRuleMode deathRule)
{
    switch (deathRule)
    {
        case DeathRuleMode::Definitive:
            return "Mort définitive";
        case DeathRuleMode::NonDefinitive:
        default:
            return "Mort non définitive";
    }
}

std::string DeathRuleRules::shortSummary(DifficultyMode difficulty, DeathRuleMode deathRule)
{
    const DeathRuleMode normalizedRule = normalizeForDifficulty(difficulty, deathRule);

    if (difficulty == DifficultyMode::Easy)
    {
        return "Facile force la mort non définitive : aucun effacement de personnage sur cette difficulté.";
    }

    if (difficulty == DifficultyMode::Lethal)
    {
        return "Léthal force la mort définitive : une vraie chute peut déplacer le personnage au registre des morts.";
    }

    if (normalizedRule == DeathRuleMode::Definitive)
    {
        return "Mort définitive activée : le personnage garde sa difficulté, mais une vraie chute peut l'effacer du registre jouable.";
    }

    return "Mort non définitive activée : les pénalités de mort existent, mais le personnage reste jouable après respawn.";
}
