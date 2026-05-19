// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Random name generator for normal NPCs and AI fighters.

#include "character/RandomNameGenerator.hpp"

#include <vector>

namespace
{
    std::string choose(const std::vector<std::string>& values, Random& random)
    {
        int index = random.between(0, static_cast<int>(values.size()) - 1);
        return values[index];
    }

    std::string buildFullName(
        const std::vector<std::string>& firstNames,
        const std::vector<std::string>& lastNames,
        Random& random
    )
    {
        return choose(firstNames, random) + " " + choose(lastNames, random);
    }
}

std::string RandomNameGenerator::generateHumanName(Random& random)
{
    static const std::vector<std::string> firstNames = {
        "Aren", "Lysa", "Kael", "Mira", "Dorian", "Eliane", "Noam", "Soren",
        "Talia", "Riven", "Maelis", "Cedrik", "Nora", "Evan", "Sylas", "Ilyan",
        "Aelia", "Garran", "Lucien", "Iris", "Marwen", "Selene", "Tobias", "Roxane",
        "Alaric", "Meline", "Rowan", "Sasha", "Erynn", "Victor", "Nelia", "Hugo"
    };

    static const std::vector<std::string> lastNames = {
        "Valbrume", "Dargent", "Clairevoie", "Noirelance", "Defer", "Hauterive",
        "Aubétoile", "Rochefort", "Lunecendre", "Sombreval", "Boisclair", "Vifacier",
        "Rougegarde", "Morneplaine", "Belorage", "Grisvent", "Dorlame", "Fauconnet",
        "Solbrun", "Marcheval", "Verrelune", "Cordacier", "Flammebourg", "Durespine"
    };

    return buildFullName(firstNames, lastNames, random);
}

std::string RandomNameGenerator::generateSemiHumanName(Random& random)
{
    static const std::vector<std::string> firstNames = {
        "Vaelyr", "Nyssiel", "Korvan", "Aelys", "Theryn", "Lyssara", "Mokai", "Zerath",
        "Kitsara", "Faelynn", "Draekor", "Selya", "Oryn", "Velka", "Shinra", "Aurek",
        "Ysolde", "Nymaris", "Kaoryn", "Sythra", "Elvyr", "Rhaelya", "Naoki", "Azhren",
        "Kalyss", "Ravhiel", "Myrrha", "Soryn", "Vaelka", "Tserin", "Ishka", "Noctalya"
    };

    static const std::vector<std::string> lastNames = {
        "Sombrelune", "Ailecendre", "Nuitclaire", "Rêveflamme", "Brumecorne", "Lamevoile",
        "Rougegivre", "Crocsolaire", "Etoilecreuse", "Oragebleu", "Feuillombre", "Vifrenard",
        "Crocdor", "Sangécaille", "Aubevoilée", "Cendrelys", "Fleurmorte", "Queueargent",
        "Ecailleblanche", "Voixdombre", "Lunefauve", "Plumenoire", "Roncesonge", "Ventmiroir"
    };

    return buildFullName(firstNames, lastNames, random);
}

std::string RandomNameGenerator::generateNonHumanName(Random& random)
{
    static const std::vector<std::string> names = {
        "Grorvak Brise-Casque", "Murgash Croc-Rouge", "Throkk Poing-de-Pierre",
        "Velkhar Noctecendre", "Zarveth Lame-Froide", "Akhrys Sang-Noir",
        "Draemir Ecailles-Brisées", "Sythrax Coeur-de-Braise", "Orvhal Gueule-d'Orage",
        "Felyx Poussière-Lune", "Nirril Cloche-Rieuse", "Bimble Rouagefin",
        "Malkor Os-Fendu", "Veyra Nuit-Sans-Fin", "Krazhul Mange-Lumière",
        "Elyssar Vitrail-d'Aube", "Thamiel Plume-Sacrée", "Aurelion Veille-Flamme"
    };

    return choose(names, random);
}

std::string RandomNameGenerator::generateNameForRace(CharacterRace race, Random& random)
{
    switch (race)
    {
        case CharacterRace::Human:
            return generateHumanName(random);

        case CharacterRace::Elf:
        case CharacterRace::DarkElf:
        case CharacterRace::Kitsune:
        case CharacterRace::Fairy:
        case CharacterRace::HalfDragon:
        case CharacterRace::Aasimar:
        case CharacterRace::Tiefling:
        case CharacterRace::Vampire:
        case CharacterRace::SemiHuman:
            return generateSemiHumanName(random);

        case CharacterRace::Dwarf:
        case CharacterRace::Gnome:
        case CharacterRace::Halfling:
        case CharacterRace::Orc:
        case CharacterRace::Demon:
        case CharacterRace::Other:
        default:
            return generateNonHumanName(random);
    }
}

std::string RandomNameGenerator::generateAnyName(Random& random)
{
    int roll = random.between(1, 100);

    if (roll <= 45)
    {
        return generateHumanName(random);
    }

    if (roll <= 80)
    {
        return generateSemiHumanName(random);
    }

    return generateNonHumanName(random);
}
