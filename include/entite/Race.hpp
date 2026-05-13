#ifndef RACE_HPP
#define RACE_HPP

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
    Inconnue
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