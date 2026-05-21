// EN: RaceCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: RaceCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Playable race catalog, racial starting bonuses and future trade modifiers.

#include "character/RaceCatalog.hpp"

#include <iostream>

std::vector<CharacterRace> RaceCatalog::getPlayableRaces()
{
    return {
        CharacterRace::Human,
        CharacterRace::Elf,
        CharacterRace::DarkElf,
        CharacterRace::Dwarf,
        CharacterRace::Gnome,
        CharacterRace::Halfling,
        CharacterRace::Tiefling,
        CharacterRace::Aasimar,
        CharacterRace::Kitsune,
        CharacterRace::Fairy,
        CharacterRace::HalfDragon,
        CharacterRace::Orc,
        CharacterRace::Vampire,
        CharacterRace::Demon,
        CharacterRace::SemiHuman
    };
}

// EN: getPlayableRaceCount declares or implements a focused behavior used by this module.
// FR: getPlayableRaceCount déclare ou implémente un comportement précis utilisé par ce module.
int RaceCatalog::getPlayableRaceCount()
{
    return static_cast<int>(getPlayableRaces().size());
}

// EN: getPlayableRaceByChoice declares or implements a focused behavior used by this module.
// FR: getPlayableRaceByChoice déclare ou implémente un comportement précis utilisé par ce module.
CharacterRace RaceCatalog::getPlayableRaceByChoice(int choice)
{
    std::vector<CharacterRace> races = getPlayableRaces();

    if (choice < 1 || choice > static_cast<int>(races.size()))
    {
        return CharacterRace::Human;
    }

    return races[choice - 1];
}

// EN: displayPlayableRaces declares or implements a focused behavior used by this module.
// FR: displayPlayableRaces déclare ou implémente un comportement précis utilisé par ce module.
void RaceCatalog::displayPlayableRaces()
{
    std::vector<CharacterRace> races = getPlayableRaces();

    for (std::size_t i = 0; i < races.size(); ++i)
    {
        CharacterRace race = races[i];
        RaceStartingBonus bonus = getStartingBonus(race);

        std::cout << (i + 1) << " : " << characterRaceToText(race) << std::endl;
        std::cout << "    Identité : " << getGameplayIdentity(race) << std::endl;
        std::cout << "    Description : " << getShortDescription(race) << std::endl;
        std::cout << "    Bonus départ : PV " << bonus.maxHpBonus
                  << " | Dégâts min " << bonus.minDamageBonus
                  << " | Dégâts max " << bonus.maxDamageBonus
                  << " | Critique " << bonus.criticalDamageBonus
                  << std::endl;

        if (race == CharacterRace::Demon)
        {
            std::cout << "    Commerce futur : certaines ventes pourront coûter plus cher à cause de ta nature démoniaque." << std::endl;
        }

        std::cout << std::endl;
    }
}

// EN: getStartingBonus declares or implements a focused behavior used by this module.
// FR: getStartingBonus déclare ou implémente un comportement précis utilisé par ce module.
RaceStartingBonus RaceCatalog::getStartingBonus(CharacterRace race)
{
    switch (race)
    {
        case CharacterRace::Human:
            return {5, 0, 0, 0};

        case CharacterRace::Elf:
            return {0, 0, 2, 1};

        case CharacterRace::DarkElf:
            return {-5, 1, 1, 3};

        case CharacterRace::Dwarf:
            return {15, 0, 0, 0};

        case CharacterRace::Gnome:
            return {-5, 0, 1, 2};

        case CharacterRace::Halfling:
            return {0, 0, 1, 2};

        case CharacterRace::Tiefling:
            return {0, 1, 2, 2};

        case CharacterRace::Aasimar:
            return {10, 0, 1, 1};

        case CharacterRace::Kitsune:
            return {-5, 0, 2, 3};

        case CharacterRace::Fairy:
            return {-10, 0, 2, 5};

        case CharacterRace::HalfDragon:
            return {10, 1, 2, 1};

        case CharacterRace::Orc:
            return {10, 3, 0, 0};

        case CharacterRace::Vampire:
            return {5, 1, 1, 2};

        case CharacterRace::Demon:
            return {5, 2, 2, 2};

        case CharacterRace::SemiHuman:
            return {5, 1, 1, 1};

        case CharacterRace::Other:
        default:
            return {0, 0, 0, 0};
    }
}

std::string RaceCatalog::getShortDescription(CharacterRace race)
{
    switch (race)
    {
        case CharacterRace::Human:
            return "Adaptable, stable, compatible avec presque toutes les voies.";

        case CharacterRace::Elf:
            return "Agile, précis, naturellement lié à la perception et à la magie fine.";

        case CharacterRace::DarkElf:
            return "Furtif, dangereux, excellent pour les critiques et les styles d'ombre.";

        case CharacterRace::Dwarf:
            return "Solide, endurant, difficile à faire tomber.";

        case CharacterRace::Gnome:
            return "Petit, malin, très adapté aux inventions et à la magie curieuse.";

        case CharacterRace::Halfling:
            return "Chanceux, discret, plus dangereux qu'il n'en a l'air.";

        case CharacterRace::Tiefling:
            return "Marqué par l'infernal, naturellement doué pour les pouvoirs risqués.";

        case CharacterRace::Aasimar:
            return "Touché par le céleste, affinité avec la lumière, la protection et la foi.";

        case CharacterRace::Kitsune:
            return "Mystique, rapide, très proche des illusions, du feu spirituel et des invocations.";

        case CharacterRace::Fairy:
            return "Très fragile physiquement, mais incroyablement connectée à la magie.";

        case CharacterRace::HalfDragon:
            return "Sang draconique, corps robuste et potentiel élémentaire élevé.";

        case CharacterRace::Orc:
            return "Puissant, frontal, très adapté aux armes lourdes et au combat brutal.";

        case CharacterRace::Vampire:
            return "Prédateur élégant, futur potentiel de vol de vie, mais faiblesse à la lumière.";

        case CharacterRace::Demon:
            return "Force sombre, présence inquiétante, mais relations commerciales et sociales compliquées.";

        case CharacterRace::SemiHuman:
            return "Origines mixtes, équilibre entre instinct, civilisation et adaptation.";

        case CharacterRace::Other:
        default:
            return "Race non classée. Son potentiel sera défini plus tard.";
    }
}

std::string RaceCatalog::getGameplayIdentity(CharacterRace race)
{
    switch (race)
    {
        case CharacterRace::Human:
            return "polyvalence";
        case CharacterRace::Elf:
            return "précision / esquive future";
        case CharacterRace::DarkElf:
            return "critique / furtivité future";
        case CharacterRace::Dwarf:
            return "résistance / durabilité future";
        case CharacterRace::Gnome:
            return "artificier / magie technique";
        case CharacterRace::Halfling:
            return "chance / discrétion";
        case CharacterRace::Tiefling:
            return "magie sombre / pactes";
        case CharacterRace::Aasimar:
            return "lumière / protection";
        case CharacterRace::Kitsune:
            return "illusion / feu spirituel / invocation";
        case CharacterRace::Fairy:
            return "magie pure / fragilité";
        case CharacterRace::HalfDragon:
            return "élémentaire / puissance hybride";
        case CharacterRace::Orc:
            return "force physique / brutalité";
        case CharacterRace::Vampire:
            return "vol de vie futur / nuit";
        case CharacterRace::Demon:
            return "puissance sombre / commerce difficile";
        case CharacterRace::SemiHuman:
            return "adaptation / identité mixte";
        case CharacterRace::Other:
        default:
            return "inconnue";
    }
}

// EN: getMerchantPurchasePricePercentage declares or implements a focused behavior used by this module.
// FR: getMerchantPurchasePricePercentage déclare ou implémente un comportement précis utilisé par ce module.
int RaceCatalog::getMerchantPurchasePricePercentage(CharacterRace race)
{
    if (race == CharacterRace::Demon)
    {
        return 125;
    }

    if (race == CharacterRace::Vampire || race == CharacterRace::Tiefling)
    {
        return 110;
    }

    if (race == CharacterRace::Human || race == CharacterRace::Halfling)
    {
        return 95;
    }

    return 100;
}

// EN: getMerchantSalePricePercentage declares or implements a focused behavior used by this module.
// FR: getMerchantSalePricePercentage déclare ou implémente un comportement précis utilisé par ce module.
int RaceCatalog::getMerchantSalePricePercentage(CharacterRace race)
{
    if (race == CharacterRace::Demon)
    {
        return 80;
    }

    if (race == CharacterRace::Vampire || race == CharacterRace::Tiefling)
    {
        return 90;
    }

    if (race == CharacterRace::Human || race == CharacterRace::Halfling)
    {
        return 105;
    }

    return 100;
}
