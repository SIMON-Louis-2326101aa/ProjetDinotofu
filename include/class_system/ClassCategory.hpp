// EN: ClassCategory.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: ClassCategory.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Defines broad gameplay families used to organize playable classes in menus.
// Description : Définit les grandes familles de gameplay utilisées pour organiser les classes jouables dans les menus.

#ifndef INCLUDE_CLASS_SYSTEM_CLASSCATEGORY_HPP
#define INCLUDE_CLASS_SYSTEM_CLASSCATEGORY_HPP

#include <string>

enum class ClassCategory
{
    Melee,
    Distance,
    Magic,
    Invocation,
    Support,
    Hybrid,
    Craft,
    Special
};

inline std::string classCategoryToText(ClassCategory category)
{
    switch (category)
    {
        case ClassCategory::Melee:
            return "Corps à corps";

        case ClassCategory::Distance:
            return "Distance";

        case ClassCategory::Magic:
            return "Magie";

        case ClassCategory::Invocation:
            return "Invocation";

        case ClassCategory::Support:
            return "Support";

        case ClassCategory::Hybrid:
            return "Hybride";

        case ClassCategory::Craft:
            return "Artisanat / technique";

        case ClassCategory::Special:
        default:
            return "Spécial";
    }
}

#endif
