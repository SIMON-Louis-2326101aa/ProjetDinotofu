// EN: CharacterRace.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CharacterRace.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file prepares playable and semi-human races for Dinotofu.
// Français : Ce fichier prépare les races jouables et semi-humaines de Dinotofu.

#ifndef INCLUDE_CHARACTER_CHARACTERRACE_HPP
#define INCLUDE_CHARACTER_CHARACTERRACE_HPP

#include <string>

enum class CharacterRace
{
    Human,
    DarkElf,
    Elf,
    Dwarf,
    Gnome,
    Halfling,
    Tiefling,
    Aasimar,
    Kitsune,
    Fairy,
    HalfDragon,
    Orc,
    Vampire,
    Demon,
    SemiHuman,
    SemiWolf,
    SemiFox,
    SemiDog,
    SemiCat,
    SemiLizard,
    SemiBird,
    Other
};

inline std::string characterRaceToText(CharacterRace race)
{
    switch (race)
    {
        case CharacterRace::Human:
            return "Humain";
        case CharacterRace::DarkElf:
            return "Elfe noir";
        case CharacterRace::Elf:
            return "Elfe";
        case CharacterRace::Dwarf:
            return "Nain";
        case CharacterRace::Gnome:
            return "Gnome";
        case CharacterRace::Halfling:
            return "Halfelin";
        case CharacterRace::Tiefling:
            return "Tieffelin";
        case CharacterRace::Aasimar:
            return "Aasimar";
        case CharacterRace::Kitsune:
            return "Kitsune";
        case CharacterRace::Fairy:
            return "Fée";
        case CharacterRace::HalfDragon:
            return "Semi-dragon";
        case CharacterRace::Orc:
            return "Orc";
        case CharacterRace::Vampire:
            return "Vampire";
        case CharacterRace::Demon:
            return "Démon";
        case CharacterRace::SemiHuman:
            return "Semi-humain";
        case CharacterRace::SemiWolf:
            return "Semi-humain loup";
        case CharacterRace::SemiFox:
            return "Semi-humain renard";
        case CharacterRace::SemiDog:
            return "Semi-humain chien";
        case CharacterRace::SemiCat:
            return "Semi-humain chat";
        case CharacterRace::SemiLizard:
            return "Semi-humain lézard";
        case CharacterRace::SemiBird:
            return "Semi-humain piaf";
        default:
            return "Autre";
    }
}

#endif
