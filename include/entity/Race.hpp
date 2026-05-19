// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ENTITY_RACE_HPP
#define INCLUDE_ENTITY_RACE_HPP

#include <string>

enum class Race
{
    Humain,
    SemiHumain,
    Elfe,
    ElfeNoir,
    Nain,
    Gnome,
    Halfelin,
    Tieffelin,
    Aasimar,
    Kitsune,
    Fee,
    SemiDragon,
    Gobelin,
    Hobgobelin,
    Orc,
    MortVivant,
    Demon,
    Ange,
    Bete,
    Dragon,
    Draconide,
    Esprit,
    Elementaire,
    Slime,
    Plante,
    Insectoide,
    Construction,
    Aberration,
    AnomalieArcanique,
    Invocation,
    Unknown
};

inline std::string raceVersTexte(Race race)
{
    switch (race)
    {
        case Race::Humain:
            return "Humain";
        case Race::SemiHumain:
            return "Semi-humain";
        case Race::Elfe:
            return "Elfe";
        case Race::ElfeNoir:
            return "Elfe noir";
        case Race::Nain:
            return "Nain";
        case Race::Gnome:
            return "Gnome";
        case Race::Halfelin:
            return "Halfelin";
        case Race::Tieffelin:
            return "Tieffelin";
        case Race::Aasimar:
            return "Aasimar";
        case Race::Kitsune:
            return "Kitsune";
        case Race::Fee:
            return "Fée";
        case Race::SemiDragon:
            return "Semi-dragon";
        case Race::Gobelin:
            return "Gobelin";
        case Race::Hobgobelin:
            return "Hobgobelin";
        case Race::Orc:
            return "Orc";
        case Race::MortVivant:
            return "Mort-vivant";
        case Race::Demon:
            return "Démon";
        case Race::Ange:
            return "Ange";
        case Race::Bete:
            return "Bête";
        case Race::Dragon:
            return "Dragon";
        case Race::Draconide:
            return "Draconide";
        case Race::Esprit:
            return "Esprit";
        case Race::Elementaire:
            return "Élémentaire";
        case Race::Slime:
            return "Slime";
        case Race::Plante:
            return "Plante";
        case Race::Insectoide:
            return "Insectoïde";
        case Race::Construction:
            return "Construction";
        case Race::Aberration:
            return "Aberration";
        case Race::AnomalieArcanique:
            return "Anomalie arcanique";
        case Race::Invocation:
            return "Invocation";
        default:
            return "Inconnue";
    }
}

#endif
