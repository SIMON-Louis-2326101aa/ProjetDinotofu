// EN: RaceCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: RaceCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Playable race catalog, racial starting bonuses and future trade modifiers.

#include "character/RaceCatalog.hpp"

#include "interface/menu/common/MessageScreen.hpp"

#include <vector>
#include <string>

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
        CharacterRace::SemiHuman,
        CharacterRace::SemiWolf,
        CharacterRace::SemiFox,
        CharacterRace::SemiDog,
        CharacterRace::SemiCat,
        CharacterRace::SemiLizard,
        CharacterRace::SemiBird
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
std::vector<std::string> RaceCatalog::getPlayableRaceDisplayLines()
{
    std::vector<std::string> lines;
    std::vector<CharacterRace> races = getPlayableRaces();

    for (std::size_t i = 0; i < races.size(); ++i)
    {
        CharacterRace race = races[i];
        RaceStartingBonus bonus = getStartingBonus(race);

        if (race == CharacterRace::Human)
        {
            lines.push_back("--- Races classiques ---");
        }
        else if (race == CharacterRace::SemiHuman)
        {
            lines.push_back("--- Semi-humains ---");
        }
        else if (race == CharacterRace::SemiWolf)
        {
            lines.push_back("--- Sous-types semi-humains ---");
            lines.push_back("    Note : ces choix évitent de tout mettre dans un seul semi-humain générique. Chaque sous-type a ses propres réactions futures.");
        }

        lines.push_back(std::to_string(i + 1) + " : " + characterRaceToText(race));
        lines.push_back("    Identité : " + getGameplayIdentity(race));
        lines.push_back("    Description : " + getShortDescription(race));
        lines.push_back(
            "    Bonus départ : PV " + std::to_string(bonus.maxHpBonus)
            + " | Dégâts min " + std::to_string(bonus.minDamageBonus)
            + " | Dégâts max " + std::to_string(bonus.maxDamageBonus)
            + " | Critique " + std::to_string(bonus.criticalDamageBonus)
        );

        lines.push_back("    " + getRaceFamilyLine(race));
        lines.push_back("    " + getInnatePassiveLine(race));
        lines.push_back("    " + getElementalAffinityLine(race));

        if (race == CharacterRace::Demon)
        {
            lines.push_back("    Commerce : certains vendeurs hésitent déjà devant ta nature démoniaque.");
        }

        lines.push_back("");
    }

    return lines;
}


// EN: displayPlayableRaces declares or implements a focused behavior used by this module.
// FR: displayPlayableRaces déclare ou implémente un comportement précis utilisé par ce module.
void RaceCatalog::displayPlayableRaces()
{
    MessageScreen::show("RACES JOUABLES", "catalog.races.playable", getPlayableRaceDisplayLines(), false);
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

        case CharacterRace::SemiWolf:
            return {8, 2, 1, 0};

        case CharacterRace::SemiFox:
            return {0, 0, 2, 3};

        case CharacterRace::SemiDog:
            return {7, 1, 1, 1};

        case CharacterRace::SemiCat:
            return {-2, 0, 2, 4};

        case CharacterRace::SemiLizard:
            return {8, 1, 1, 0};

        case CharacterRace::SemiBird:
            return {-4, 0, 3, 3};

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
            return "Prédateur élégant, attiré par le sang et fragile face à la lumière.";

        case CharacterRace::Demon:
            return "Force sombre, présence inquiétante, mais relations commerciales et sociales compliquées.";

        case CharacterRace::SemiHuman:
            return "Catégorie large de peuples semi-humains : instinct animal, identité sociale et adaptation.";

        case CharacterRace::SemiWolf:
            return "Semi-humain loup, endurant, pisteur et très bon pour lire une route ou une menace.";

        case CharacterRace::SemiFox:
            return "Semi-humain renard, rusé, discret, proche des pistes nocturnes sans être un kitsune mystique.";

        case CharacterRace::SemiDog:
            return "Semi-humain chien, loyal, robuste et très fiable pour suivre une trace ou protéger un groupe.";

        case CharacterRace::SemiCat:
            return "Semi-humain chat, agile, prudent, doté d'une bonne vision nocturne et d'un instinct d'évitement.";

        case CharacterRace::SemiLizard:
            return "Semi-humain lézard, peau écailleuse, bonne tenue à la chaleur mais froid plus pénible.";

        case CharacterRace::SemiBird:
            return "Semi-humain piaf, léger, mobile et perceptif, mais ses plumes craignent fortement les flammes.";

        case CharacterRace::Other:
        default:
            return "Race non classée. Son potentiel reste enveloppé de brouillard.";
    }
}

std::string RaceCatalog::getGameplayIdentity(CharacterRace race)
{
    switch (race)
    {
        case CharacterRace::Human:
            return "polyvalence";
        case CharacterRace::Elf:
            return "précision / esquive instinctive";
        case CharacterRace::DarkElf:
            return "critique / furtivité naturelle";
        case CharacterRace::Dwarf:
            return "résistance / endurance physique";
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
            return "vol de vie latent / nuit";
        case CharacterRace::Demon:
            return "puissance sombre / commerce difficile";
        case CharacterRace::SemiHuman:
            return "adaptation / identité mixte";
        case CharacterRace::SemiWolf:
            return "piste / endurance / meute";
        case CharacterRace::SemiFox:
            return "ruse / discrétion / route nocturne";
        case CharacterRace::SemiDog:
            return "loyauté / flair / protection";
        case CharacterRace::SemiCat:
            return "agilité / nuit / esquive";
        case CharacterRace::SemiLizard:
            return "écailles / chaleur / froid difficile";
        case CharacterRace::SemiBird:
            return "mobilité / perception / plumes inflammables";
        case CharacterRace::Other:
        default:
            return "inconnue";
    }
}


bool RaceCatalog::isSemiHumanFamily(CharacterRace race)
{
    return race == CharacterRace::SemiHuman
        || race == CharacterRace::SemiWolf
        || race == CharacterRace::SemiFox
        || race == CharacterRace::SemiDog
        || race == CharacterRace::SemiCat
        || race == CharacterRace::SemiLizard
        || race == CharacterRace::SemiBird
        || race == CharacterRace::HalfDragon;
}

std::string RaceCatalog::getRaceFamilyLine(CharacterRace race)
{
    if (race == CharacterRace::HalfDragon)
    {
        return "Famille : hybride draconique, proche de la logique semi-humaine mais traité comme lignée spéciale.";
    }

    if (race == CharacterRace::SemiHuman)
    {
        return "Famille : semi-humain générique. Plus tard, ce choix pourra ouvrir un sous-type plus précis.";
    }

    if (isSemiHumanFamily(race))
    {
        return "Famille : semi-humain / sous-type animal. Les dialogues, morphologies et équipements pourront réagir à ce sous-type.";
    }

    return "Famille : race principale.";
}

bool RaceCatalog::hasInnateNightVision(CharacterRace race)
{
    return race == CharacterRace::DarkElf
        || race == CharacterRace::Kitsune
        || race == CharacterRace::Vampire
        || race == CharacterRace::Demon
        || race == CharacterRace::SemiCat
        || race == CharacterRace::SemiFox;
}

int RaceCatalog::getEnvironmentalTemperatureScore(CharacterRace race, const std::string& hazard)
{
    int score = 0;

    // Kitsune et semi-dragon passent par leurs passifs explicites : adaptation de température + faible résistance feu.
    // On évite de doubler leur score ici pour qu'ils restent aidés, mais pas gratuits en zones extrêmes.

    if (race == CharacterRace::Tiefling || race == CharacterRace::Demon)
    {
        if (hazard == "chaleur" || hazard == "feu") score += 2;
    }

    if (race == CharacterRace::SemiLizard)
    {
        if (hazard == "chaleur") score += 2;
        if (hazard == "feu") score += 1;
        if (hazard == "froid") score -= 1;
    }

    if (race == CharacterRace::SemiWolf || race == CharacterRace::SemiDog)
    {
        if (hazard == "froid") score += 1;
    }

    if (race == CharacterRace::Dwarf)
    {
        if (hazard == "froid") score += 1;
    }

    if (race == CharacterRace::Vampire)
    {
        if (hazard == "froid") score += 1;
        if (hazard == "chaleur" || hazard == "feu") score -= 1;
    }

    if (hasFireWeakness(race) && (hazard == "chaleur" || hazard == "feu"))
    {
        score -= 1;
    }

    return score;
}

bool RaceCatalog::hasFireWeakness(CharacterRace race)
{
    return race == CharacterRace::Fairy
        || race == CharacterRace::Vampire
        || race == CharacterRace::SemiBird;
}

std::string RaceCatalog::getElementalAffinityLine(CharacterRace race)
{
    switch (race)
    {
        case CharacterRace::Kitsune:
            return "Affinités : faible résistance au feu/chaleur et aux températures tempérées difficiles ; pas assez pour un volcan extrême sans équipement.";
        case CharacterRace::HalfDragon:
            return "Affinités : sang draconique, faible résistance feu/chaleur/froid et meilleure endurance météo ; les zones extrêmes demandent quand même mieux.";
        case CharacterRace::Tiefling:
            return "Affinités : résistance infernale au feu et à la chaleur, utile aussi contre les brûlures en combat.";
        case CharacterRace::Demon:
            return "Affinités : résistance sombre au feu, à la chaleur et à plusieurs altérations, mais présence sociale difficile.";
        case CharacterRace::Dwarf:
            return "Affinités : corps dense, meilleure tenue au froid, au poison et au saignement.";
        case CharacterRace::Fairy:
            return "Affinités : magie très vive, mais faiblesse aux flammes sur les ailes/plumes fines ; les brûlures mordent plus fort.";
        case CharacterRace::Vampire:
            return "Affinités : très bon dans la nuit et le froid, mais vulnérable au feu et aux fortes chaleurs.";
        case CharacterRace::Aasimar:
            return "Affinités : lumière protectrice, petite résistance au poison et aux flammes légères.";
        case CharacterRace::SemiWolf:
            return "Affinités : flair de pisteur, meilleure endurance au froid léger et aux longues routes.";
        case CharacterRace::SemiFox:
            return "Affinités : flair rusé et vision nocturne légère, très utile pour les sorties prudentes.";
        case CharacterRace::SemiDog:
            return "Affinités : flair loyal, bonne endurance et meilleure tenue au froid léger.";
        case CharacterRace::SemiCat:
            return "Affinités : vision nocturne, réflexes de survie et discrétion ; peu de grosses résistances élémentaires.";
        case CharacterRace::SemiLizard:
            return "Affinités : écailles et chaleur, légère protection contre le feu faible, mais froid plus difficile.";
        case CharacterRace::SemiBird:
            return "Affinités : grande perception et mobilité, mais faiblesse au feu à cause des plumes.";
        case CharacterRace::Orc:
            return "Affinités : marche forcée et endurance physique ; peu sensible aux saignements simples.";
        default:
            return "Affinités : pas de résistance ou faiblesse élémentaire majeure pour l'instant.";
    }
}

std::string RaceCatalog::getInnatePassiveLine(CharacterRace race)
{
    if (race == CharacterRace::Kitsune)
    {
        return "Passif racial : Vision nocturne, adaptation légère aux températures et petite résistance au feu spirituel. Cette résistance compte aussi contre les brûlures de combat, mais reste légère.";
    }

    if (race == CharacterRace::HalfDragon)
    {
        return "Passif racial : Sang draconique d'endurance, adaptation légère aux températures et petite résistance au feu. Cette résistance aide aussi contre les attaques de feu, sans annuler les biomes extrêmes.";
    }

    if (hasInnateNightVision(race))
    {
        if (race == CharacterRace::SemiCat)
        {
            return "Passif racial : Vision nocturne féline. Les sorties de nuit restent dangereuses, mais les ombres bougent moins vite que tes yeux.";
        }
        if (race == CharacterRace::SemiFox)
        {
            return "Passif racial : Vision nocturne de renard et flair rusé. Très utile pour les pistes secondaires et les retours prudents.";
        }
        return "Passif racial : Vision nocturne de base. Les sorties de nuit restent dangereuses, mais cette race lit mieux les ombres.";
    }

    if (race == CharacterRace::SemiWolf)
    {
        return "Passif racial : Flair de meute. Les traces, odeurs et pistes de chasse sont plus faciles à lire, surtout en forêt ou sur route sauvage.";
    }

    if (race == CharacterRace::SemiDog)
    {
        return "Passif racial : Flair loyal. Les missions d'escorte, de recherche et de protection profitent d'un instinct fiable.";
    }

    if (race == CharacterRace::SemiLizard)
    {
        return "Passif racial : Écailles tempérées. La chaleur et le feu faible passent mieux, mais le froid mord plus vite. Kanadé risque aussi de mal prendre certaines comparaisons.";
    }

    if (race == CharacterRace::SemiBird)
    {
        return "Passif racial : Vue des hauteurs. Meilleure lecture des routes et des dangers ouverts, mais les plumes craignent les flammes.";
    }

    if (race == CharacterRace::Dwarf || race == CharacterRace::Gnome)
    {
        return "Passif racial : Sens des galeries. Les mines, ruines et zones rocheuses sont plus faciles à lire en exploration.";
    }

    if (race == CharacterRace::Elf)
    {
        return "Passif racial : Perception elfique. Les traces, pistes et détails naturels ressortent mieux pendant les sorties.";
    }

    if (race == CharacterRace::Fairy || race == CharacterRace::Aasimar)
    {
        if (race == CharacterRace::Fairy)
        {
            return "Passif racial : Sens magique léger, mais ailes très vulnérables aux flammes. Les lieux chargés de mana donnent moins vite de mauvaises surprises, le feu en donne davantage.";
        }
        return "Passif racial : Sens magique léger. Les lieux chargés de mana ou de lumière donnent moins vite de mauvaises surprises.";
    }

    if (race == CharacterRace::Halfling)
    {
        return "Passif racial : Pas chanceux. Les petits accidents de route ont un peu moins de chances de tourner mal.";
    }


    if (race == CharacterRace::SemiBird)
    {
        return "Passif racial : Sens des hauteurs. Les falaises, routes ouvertes et zones aériennes donnent parfois de meilleurs appuis.";
    }

    if (race == CharacterRace::Orc)
    {
        return "Passif racial : Marche forcée orque. Les trajets physiques restent pénibles, mais la race encaisse mieux les longues distances.";
    }

    return "Passif racial prévu : identité surtout sociale/statistique pour l'instant, avec évolutions possibles plus tard.";
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

    if (race == CharacterRace::Human || race == CharacterRace::Halfling || race == CharacterRace::SemiDog)
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

    if (race == CharacterRace::Human || race == CharacterRace::Halfling || race == CharacterRace::SemiFox)
    {
        return 105;
    }

    return 100;
}
