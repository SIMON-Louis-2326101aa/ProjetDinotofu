// EN: DeathRuleRules.hpp centralizes character death-rule forcing and labels.
// FR: DeathRuleRules.hpp centralise les règles forcées et les libellés de mort.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_PROGRESSION_DEATHRULERULES_HPP
#define INCLUDE_PROGRESSION_DEATHRULERULES_HPP

#include "progression/DeathRuleMode.hpp"
#include "progression/DifficultyMode.hpp"

#include <string>

class DeathRuleRules
{
public:
    static DeathRuleMode normalizeForDifficulty(DifficultyMode difficulty, DeathRuleMode requestedRule);
    static DeathRuleMode defaultForDifficulty(DifficultyMode difficulty);
    static bool isChoiceForced(DifficultyMode difficulty);
    static bool isDefinitive(DifficultyMode difficulty, DeathRuleMode deathRule);
    static std::string toSaveText(DeathRuleMode deathRule);
    static DeathRuleMode fromSaveText(const std::string& value, DifficultyMode difficulty);
    static std::string displayName(DeathRuleMode deathRule);
    static std::string shortSummary(DifficultyMode difficulty, DeathRuleMode deathRule);
};

#endif
