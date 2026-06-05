// EN: TitleCatalog.hpp centralizes character title definitions and reveal rules.
// FR: TitleCatalog.hpp centralise les définitions de titres de personnage et les règles de révélation.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_PROGRESSION_TITLECATALOG_HPP
#define INCLUDE_PROGRESSION_TITLECATALOG_HPP

#include <string>
#include <vector>

namespace TitleCatalog
{
    struct TitleDefinition
    {
        std::string name;
        std::string category;
        std::string unlockHint;
        std::string effect;
        bool visibleBeforeUnlock = false;
        bool requiredForUltimate = true;
    };

    const std::vector<TitleDefinition>& availableTitleDefinitions();
    const TitleDefinition* findTitleDefinition(const std::string& titleName);
    std::string unlockHintFor(const std::string& titleName);
    std::string effectFor(const std::string& titleName);
    bool isVisibleBeforeUnlock(const std::string& titleName);
    bool hasAllUltimatePrerequisites(const std::vector<std::string>& ownedTitles);
}

#endif
