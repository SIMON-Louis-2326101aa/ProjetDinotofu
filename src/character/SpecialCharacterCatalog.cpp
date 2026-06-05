// EN: SpecialCharacterCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file prepares special semi-human characters for Dinotofu.
// Français : Ce fichier prépare les personnages spéciaux semi-humains de Dinotofu.

#include "character/SpecialCharacterCatalog.hpp"

#include <algorithm>
#include <cctype>
#include "interface/menu/common/MessageScreen.hpp"

#include <utility>
#include <vector>

std::vector<SpecialCharacter> SpecialCharacterCatalog::getAllSpecialCharacters()
{
    return {
        SpecialCharacter(
            "Matt (PRO)",
            CharacterRace::Human,
            "Guerrier",
            "Référence Wii Sports, adversaire spécial intelligent avec bonus natifs.",
            "Matt (PRO) refuse presque toujours d'être incarné. Il sert de repère, de rival et de petite humiliation potentielle.",
            false,
            true,
            "",
            8
        ),
        SpecialCharacter(
            "Hazak",
            CharacterRace::DarkElf,
            "Assassin",
            "Assassin elfe noir, furtivité, vitesse et coups précis.",
            "Hazak possède déjà son histoire, ses cicatrices et son rapport étrange avec la mort.",
            true,
            false,
            "08/11/2024",
            5
        ),
        SpecialCharacter(
            "Mattzelda",
            CharacterRace::Human,
            "Colosse",
            "Colosse humain, présence lourde et résistance énorme.",
            "Mattzelda transforme l'arène en mur. Le contourner est souvent plus réaliste que le briser.",
            true,
            false,
            "15/07/2005",
            4
        ),
        SpecialCharacter(
            "Aoi",
            CharacterRace::Kitsune,
            "Mage flame",
            "Future mage flame, invocatrice kitsune avec katana.",
            "Aoi mélange flammes, invocations, agilité kitsune et une lame qu'il vaut mieux ne pas ignorer.",
            true,
            false,
            "01/12/2024",
            4
        ),
        SpecialCharacter(
            "Kanadé",
            CharacterRace::HalfDragon,
            "Mage zodiacal",
            "Semi-dragonne, sorts aléatoires liés aux 13 signes du zodiaque. Déteste qu'on la confonde avec un lézard.",
            "Kanadé ne contrôle pas toujours ce qui tombe du ciel, mais le ciel répond quand même. La traiter de lézard est une très mauvaise idée.",
            true,
            false,
            "06/12/2024",
            4
        ),
        SpecialCharacter(
            "Fail",
            CharacterRace::Fairy,
            "Mage fou",
            "Fée mage fou, magie variée, parfois unique, parfois incompréhensible.",
            "Fail peut rater un plan, réussir une catastrophe et appeler ça une stratégie.",
            true,
            false,
            "10/10/2024",
            4
        ),
        SpecialCharacter(
            "Trexof",
            CharacterRace::Human,
            "Assassin",
            "Assassin humain, analyste instinctif des failles, bonus légers.",
            "Trexof est fait pour apparaître rarement, lire les failles du terrain et rappeler que les humains peuvent aussi être dangereux.",
            true,
            false,
            "09/09/2005",
            5
        ),
        SpecialCharacter(
            "Skuro",
            CharacterRace::Human,
            "Briseur lourd",
            "Tank à épée lourde deux mains, précision instable mais dégâts massifs.",
            "Skuro ne frappe pas souvent. Le problème, c'est quand il touche.",
            true,
            false,
            "06/12/2024",
            4
        ),
        SpecialCharacter(
            "Sanctus",
            CharacterRace::Aasimar,
            "Tank sacré",
            "Tank semi-mage, protection, entrave et croyances liées à un dieu de la lumière.",
            "Sanctus ne vient pas seulement gagner. Il vient imposer une foi, un mur et un jugement.",
            true,
            false,
            "06/12/2024",
            4
        ),
        SpecialCharacter(
            "Hestia",
            CharacterRace::Human,
            "Mage",
            "Stats presque nulles sauf magie, sorts renforcés à chaque utilisation, dôme de protection max dès le début.",
            "Hestia semble fragile jusqu'au moment où la magie commence à apprendre à travers elle.",
            true,
            false,
            "11/11/1111",
            3
        ),
        SpecialCharacter(
            "Fire Flight",
            CharacterRace::Human,
            "Guerrier",
            "FireFlight : stats modestes, mais sous 50% PV il esquive mieux et critique presque toujours.",
            "Fire Flight n'est pas le plus fort sur le papier. Malheureusement, le papier ne code pas les règles. Cette identité appartient au test final, pas à la création de personnage.",
            false,
            true,
            "",
            2
        ),
        SpecialCharacter(
            "Louis",
            CharacterRace::Human,
            "Artificier",
            "Artificier précis, plusieurs projectiles par attaque.",
            "Louis cherche surtout des alliés. Il est naïf, gentil de base, mais ses projectiles n'attendent pas toujours la fin de la discussion.",
            true,
            false,
            "01/02/2005",
            4
        ),
        SpecialCharacter(
            "Henrique",
            CharacterRace::Human,
            "Chevalier",
            "Chevalier frontal, difficile à garder au sol quand il refuse de tomber.",
            "Henrique fonce dans le tas avec une foi simple : tomber une fois n'est pas une raison suffisante pour s'arrêter.",
            true,
            false,
            "29/11/2024",
            4
        )
    };
}

// EN: isProtectedName declares or implements a focused behavior used by this module.
// FR: isProtectedName déclare ou implémente un comportement précis utilisé par ce module.
bool SpecialCharacterCatalog::isProtectedName(const std::string& name)
{
    SpecialCharacter unused;
    return findByName(name, unused);
}

// EN: findByName declares or implements a focused behavior used by this module.
// FR: findByName déclare ou implémente un comportement précis utilisé par ce module.
bool SpecialCharacterCatalog::findByName(const std::string& name, SpecialCharacter& result)
{
    std::string normalizedInput = normalizeName(name);

    for (const SpecialCharacter& character : getAllSpecialCharacters())
    {
        if (normalizeName(character.getName()) == normalizedInput)
        {
            result = character;
            return true;
        }
    }

    return false;
}

// EN: createRandomSpecialOpponent declares or implements a focused behavior used by this module.
// FR: createRandomSpecialOpponent déclare ou implémente un comportement précis utilisé par ce module.
SpecialCharacter SpecialCharacterCatalog::createRandomSpecialOpponent(Random& random)
{
    std::vector<SpecialCharacter> characters = getAllSpecialCharacters();
    int totalWeight = 0;

    for (const SpecialCharacter& character : characters)
    {
        // Matt (PRO) is non-playable for the player, but he can still appear as a special arena opponent.
        // Matt (PRO) est non jouable pour le joueur, mais il peut quand même apparaître comme adversaire spécial d'arène.
        totalWeight += character.getSpawnWeight();
    }

    if (totalWeight <= 0)
    {
        return characters.front();
    }

    int roll = random.between(1, totalWeight);
    int currentWeight = 0;

    for (const SpecialCharacter& character : characters)
    {
        currentWeight += character.getSpawnWeight();

        if (roll <= currentWeight)
        {
            return character;
        }
    }

    return characters.front();
}

// EN: displaySpecialCharactersRoadmap declares or implements a focused behavior used by this module.
// FR: displaySpecialCharactersRoadmap déclare ou implémente un comportement précis utilisé par ce module.
std::vector<std::string> SpecialCharacterCatalog::getSpecialCharactersRoadmapLines()
{
    std::vector<std::string> lines;

    for (const SpecialCharacter& character : getAllSpecialCharacters())
    {
        lines.push_back(
            "- " + character.getName()
            + " | Race : " + character.getRaceText()
            + " | Classe native : " + character.getNativeClass()
        );
        lines.push_back("  Style : " + character.getCombatStyle());

        if (character.isPermanentlyNonPlayable())
        {
            lines.push_back("  Jouable : non, identité réservée.");
        }
        else if (character.canBePlayedWithSpecialDate())
        {
            lines.push_back("  Jouable : seulement avec validation de date spéciale.");
        }

        lines.push_back("");
    }

    return lines;
}


// EN: displaySpecialCharactersRoadmap declares or implements a focused behavior used by this module.
// FR: displaySpecialCharactersRoadmap déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterCatalog::displaySpecialCharactersRoadmap()
{
    MessageScreen::show(
        "PERSONNAGES SPÉCIAUX",
        "catalog.special_characters.roadmap",
        getSpecialCharactersRoadmapLines(),
        false
    );
}

std::string SpecialCharacterCatalog::normalizeName(const std::string& name)
{
    std::string normalized = name;

    std::transform(
        normalized.begin(),
        normalized.end(),
        normalized.begin(),
        // EN: [] declares or implements a focused behavior used by this module.
        // FR: [] déclare ou implémente un comportement précis utilisé par ce module.
        [](unsigned char character)
        {
            return static_cast<char>(std::tolower(character));
        }
    );

    const std::vector<std::pair<std::string, std::string>> replacements = {
        {"é", "e"}, {"è", "e"}, {"ê", "e"}, {"ë", "e"},
        {"à", "a"}, {"â", "a"}, {"ä", "a"},
        {"î", "i"}, {"ï", "i"},
        {"ô", "o"}, {"ö", "o"},
        {"ù", "u"}, {"û", "u"}, {"ü", "u"},
        {"ç", "c"}
    };

    for (const auto& replacement : replacements)
    {
        std::string::size_type position = 0;

        while ((position = normalized.find(replacement.first, position)) != std::string::npos)
        {
            normalized.replace(position, replacement.first.size(), replacement.second);
            position += replacement.second.size();
        }
    }

    std::string compact;

    for (unsigned char character : normalized)
    {
        if (std::isalnum(character))
        {
            compact += static_cast<char>(character);
        }
    }

    return compact;
}
