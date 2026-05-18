// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ENTITY_RACE_HPP
#define INCLUDE_ENTITY_RACE_HPP

#include <string>

enum class Race
{
    Humain,
    Gobelin,
    Orc,
    MortVivant,
    Demon,
    Ange,
    Bete,
    Dragon,
    Esprit,
    Unknown
};

inline std::string raceVersTexte(Race race)
{
    switch (race)
    {
        case Race::Humain:
            return "Humain";
        case Race::Gobelin:
            return "Gobelin";
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
        case Race::Esprit:
            return "Esprit";
        default:
            return "Inconnue";
    }
}

#endif
