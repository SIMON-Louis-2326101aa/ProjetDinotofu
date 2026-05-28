// EN: AdventurerGroupEncounter.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: AdventurerGroupEncounter.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file prepares random adventurer group encounters for PvE-like battles.
// Français : Ce fichier prépare les rencontres de groupes d'aventuriers aléatoires pour les combats façon PvE.
// Description: Creates human and semi-human opponent groups using random fighters or special character groups.

#include "combat/encounter/AdventurerGroupEncounter.hpp"

#include "character/CharacterRace.hpp"
#include "character/RandomCharacterGenerator.hpp"
#include "character/SpecialCharacterCatalog.hpp"
#include "character/SpecialCharacterDialogueCatalog.hpp"
#include "character/relationship/SpecialCharacterGroupDialogueCatalog.hpp"
#include "character/SpecialCharacterNativeBonus.hpp"
#include "class_system/ClassCatalog.hpp"
#include "combat/encounter/SpecialEncounterRules.hpp"
#include "combat/action/SpecialCombatEffects.hpp"
#include "core/Console.hpp"
#include "entity/Race.hpp"
#include "progression/DifficultyMode.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <algorithm>
#include <iostream>

namespace
{

void showEncounterScreen(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        bool waitAndClear = true
    )
    {
        if (!lines.empty())
        {
            MessageScreen::show(title, screenId, lines, waitAndClear);
        }
    }

    // EN: convertCharacterRaceToEntityRace declares or implements a focused behavior used by this module.
    // FR: convertCharacterRaceToEntityRace déclare ou implémente un comportement précis utilisé par ce module.
    Race convertCharacterRaceToEntityRace(CharacterRace race)
    {
        switch (race)
        {
            case CharacterRace::Human:
                return Race::Humain;
            case CharacterRace::DarkElf:
                return Race::ElfeNoir;
            case CharacterRace::Elf:
                return Race::Elfe;
            case CharacterRace::Dwarf:
                return Race::Nain;
            case CharacterRace::Gnome:
                return Race::Gnome;
            case CharacterRace::Halfling:
                return Race::Halfelin;
            case CharacterRace::Tiefling:
                return Race::Tieffelin;
            case CharacterRace::Aasimar:
                return Race::Aasimar;
            case CharacterRace::Kitsune:
                return Race::Kitsune;
            case CharacterRace::Fairy:
                return Race::Fee;
            case CharacterRace::HalfDragon:
                return Race::SemiDragon;
            case CharacterRace::Orc:
                return Race::Orc;
            case CharacterRace::Vampire:
                return Race::Demon;
            case CharacterRace::Demon:
                return Race::Demon;
            case CharacterRace::SemiHuman:
                return Race::SemiHumain;
            default:
                return Race::SemiHumain;
        }
    }

    // EN: scaleMaxHp declares or implements a focused behavior used by this module.
    // FR: scaleMaxHp déclare ou implémente un comportement précis utilisé par ce module.
    int scaleMaxHp(int baseMaxHp, int encounterLevel)
    {
        return baseMaxHp + ((encounterLevel - 1) * 12);
    }

    // EN: scaleMinDamage declares or implements a focused behavior used by this module.
    // FR: scaleMinDamage déclare ou implémente un comportement précis utilisé par ce module.
    int scaleMinDamage(int baseMinDamage, int encounterLevel)
    {
        return baseMinDamage + ((encounterLevel - 1) / 2);
    }

    // EN: scaleMaxDamage declares or implements a focused behavior used by this module.
    // FR: scaleMaxDamage déclare ou implémente un comportement précis utilisé par ce module.
    int scaleMaxDamage(int baseMaxDamage, int encounterLevel)
    {
        return baseMaxDamage + (encounterLevel - 1);
    }

    // EN: scaleCriticalDamage declares or implements a focused behavior used by this module.
    // FR: scaleCriticalDamage déclare ou implémente un comportement précis utilisé par ce module.
    int scaleCriticalDamage(int baseCriticalDamage, int encounterLevel)
    {
        return baseCriticalDamage + ((encounterLevel - 1) * 2);
    }

    // EN: containsName declares or implements a focused behavior used by this module.
    // FR: containsName déclare ou implémente un comportement précis utilisé par ce module.
    bool containsName(const std::vector<std::string>& names, const std::string& expectedName)
    {
        return std::find(names.begin(), names.end(), expectedName) != names.end();
    }

    std::string getRandomSpecialNameFromAllowed(
        Random& random,
        const std::vector<std::string>& forbiddenNames,
        const std::vector<std::string>& allowedNames
    )
    {
        std::vector<std::string> availableNames;

        for (const std::string& allowedName : allowedNames)
        {
            if (!containsName(forbiddenNames, allowedName))
            {
                availableNames.push_back(allowedName);
            }
        }

        if (availableNames.empty())
        {
            return "Hazak";
        }

        int index = random.between(0, static_cast<int>(availableNames.size()) - 1);
        return availableNames[index];
    }

    void addRandomSkuroCompatibleSpecialName(
        Random& random,
        std::vector<std::string>& names
    )
    {
        // English: Skuro only appears with specific special characters for now.
        // Français : Pour l'instant, Skuro n'apparaît qu'avec des personnages spéciaux précis.
        std::vector<std::string> skuroCompatibleNames = {
            "Hazak",
            "Henrique",
            "Aoi",
            "Fail",
            "Kanadé"
        };

        std::string generatedName = getRandomSpecialNameFromAllowed(
            random,
            names,
            skuroCompatibleNames
        );

        names.push_back(generatedName);
    }
}

EnemyCombatQueue AdventurerGroupEncounter::createRandomGroupForPlayer(
    const Player& player,
    Random& random
)
{
    int roll = random.between(1, 100);

    if (roll <= 25)
    {
        return createSpecialGroup(player, random);
    }

    return createClassicRandomGroup(player, random);
}

// EN: displayGroupEncounterIntroduction declares or implements a focused behavior used by this module.
// FR: displayGroupEncounterIntroduction déclare ou implémente un comportement précis utilisé par ce module.
void AdventurerGroupEncounter::displayGroupEncounterIntroduction()
{
    Console::clear();

    MessageScreen::show(
        "RENCONTRE D'AVENTURIERS",
        "combat.encounter.adventurer_group.intro",
        {
            "Tu ne tombes pas sur une vague de monstres.",
            "Cette fois, l'arène appelle un groupe d'aventuriers.",
            "Ils ne sont pas forcément là pour te tuer, mais ils ne sont pas venus pour applaudir non plus."
        }
    );
}

EnemyCombatQueue AdventurerGroupEncounter::createClassicRandomGroup(
    const Player& player,
    Random& random
)
{
    EnemyCombatQueue queue;
    int groupSize = random.between(2, 3);
    int encounterLevel = player.getLevel();

    MessageScreen::show(
        "GROUPE CLASSIQUE",
        "combat.encounter.adventurer_group.classic",
        {
            "Un groupe d'aventuriers inconnus entre dans l'arène.",
            "Noms, races et classes sont générés par l'arène.",
            "Aucun personnage spécial ici.",
            "Taille du groupe : " + std::to_string(groupSize)
        }
    );

    for (int i = 0; i < groupSize; ++i)
    {
        Player opponent = RandomCharacterGenerator::generateClassicOpponent(random);
        addPlayerAsOpponent(queue, opponent, encounterLevel);
    }

    queue.initializeFrontLine();
    return queue;
}

EnemyCombatQueue AdventurerGroupEncounter::createSpecialGroup(
    const Player& player,
    Random& random
)
{
    EnemyCombatQueue queue;
    int encounterLevel = player.getLevel();
    int groupChoice = random.between(1, 12);
    std::vector<std::string> names;

    switch (groupChoice)
    {
        case 1:
            names = {"Hazak", "Henrique"};
            break;

        case 2:
            names = {"Mattzelda", "Louis", "Trexof"};
            break;

        case 3:
            names = {"Aoi", "Kanadé", "Sanctus"};
            break;

        case 4:
            names = {"Hazak", "Hestia"};
            break;

        case 5:
            names = {"Fail", "Hazak"};
            break;

        case 6:
            names = {"Skuro"};
            addRandomSkuroCompatibleSpecialName(random, names);
            break;

        case 7:
            names = {"Skuro"};
            addRandomSkuroCompatibleSpecialName(random, names);
            addRandomSkuroCompatibleSpecialName(random, names);
            break;

        case 8:
            names = {"Fire Flight", "Matt (PRO)"};
            break;

        case 9:
            names = {"Hestia", "Sanctus", "Hazak"};
            break;

        case 10:
            names = {"Fail", "Aoi", "Kanadé"};
            break;

        case 11:
            names = {"Louis", "Fire Flight", "Trexof"};
            break;

        case 12:
        default:
            names = {"Henrique", "Mattzelda", "Skuro"};
            break;
    }

    announceSpecialGroup(names);
    announceRelationshipBonus(names);
    SpecialCombatEffects::registerSpecialGroupContext(names);

    for (const std::string& name : names)
    {
        addSpecialCharacterAsOpponent(queue, name, encounterLevel, names);
    }

    queue.initializeFrontLine();
    return queue;
}

void AdventurerGroupEncounter::addPlayerAsOpponent(
    EnemyCombatQueue& queue,
    const Player& opponent,
    int encounterLevel
)
{
    Monster monster(
        opponent.getName(),
        opponent.getType(),
        convertCharacterRaceToEntityRace(opponent.getRace()),
        encounterLevel,
        scaleMaxHp(opponent.getMaxHp(), encounterLevel),
        scaleMinDamage(opponent.getMinDamage(), encounterLevel),
        scaleMaxDamage(opponent.getMaxDamage(), encounterLevel),
        scaleCriticalDamage(opponent.getCriticalDamage(), encounterLevel),
        opponent.getHealingPotionCount(),
        opponent.getDamagePotionCount(),
        false,
        true,
        false
    );

    queue.addWaitingEnemy(monster);
}

void AdventurerGroupEncounter::addSpecialCharacterAsOpponent(
    EnemyCombatQueue& queue,
    const std::string& characterName,
    int encounterLevel,
    const std::vector<std::string>& groupNames
)
{
    SpecialCharacter character;

    if (!SpecialCharacterCatalog::findByName(characterName, character))
    {
        return;
    }

    Player opponent(
        character.getName(),
        ClassCatalog::createClassByName(character.getNativeClass())
    );

    opponent.setRace(character.getRace());
    opponent.initializeStarterInventory(DifficultyMode::Normal);

    SpecialCharacterNativeBonus::applyForSpecialCharacter(
        opponent,
        character
    );

    applyRelationshipBonus(opponent, characterName, groupNames);

    addPlayerAsOpponent(queue, opponent, encounterLevel);
}

void AdventurerGroupEncounter::applyRelationshipBonus(
    Player& opponent,
    const std::string& characterName,
    const std::vector<std::string>& groupNames
)
{
    bool hasHazak = containsName(groupNames, "Hazak");
    bool hasHestia = containsName(groupNames, "Hestia");
    bool hasHenrique = containsName(groupNames, "Henrique");
    bool hasMattzelda = containsName(groupNames, "Mattzelda");
    bool hasLouis = containsName(groupNames, "Louis");
    bool hasTrexof = containsName(groupNames, "Trexof");
    bool hasAoi = containsName(groupNames, "Aoi");
    bool hasKanade = containsName(groupNames, "Kanadé");
    bool hasSanctus = containsName(groupNames, "Sanctus");
    bool hasFail = containsName(groupNames, "Fail");
    bool hasSkuro = containsName(groupNames, "Skuro");

    if (hasHazak && hasHestia)
    {
        if (characterName == "Hazak") opponent.applyFlatStatBonus(24, 2, 4, 6);
        if (characterName == "Hestia") opponent.applyFlatStatBonus(14, 0, 2, 4);
    }

    if (hasHazak && hasHenrique)
    {
        if (characterName == "Hazak") opponent.applyFlatStatBonus(8, 2, 3, 4);
        if (characterName == "Henrique") opponent.applyFlatStatBonus(28, 1, 3, 2);
    }

    if (hasMattzelda && hasLouis && hasTrexof)
    {
        if (characterName == "Mattzelda") opponent.applyFlatStatBonus(30, 1, 3, 0);
        if (characterName == "Louis") opponent.applyFlatStatBonus(12, 1, 4, 2);
        if (characterName == "Trexof") opponent.applyFlatStatBonus(8, 3, 5, 6);
    }

    if (hasAoi && hasKanade && hasSanctus)
    {
        if (characterName == "Sanctus") opponent.applyFlatStatBonus(34, 0, 2, 0);
        if (characterName == "Aoi") opponent.applyFlatStatBonus(10, 1, 4, 4);
        if (characterName == "Kanadé") opponent.applyFlatStatBonus(6, 2, 6, 7);
    }

    if (hasAoi && hasKanade && hasFail)
    {
        if (characterName == "Fail") opponent.applyFlatStatBonus(8, 1, 8, 9);
        if (characterName == "Aoi") opponent.applyFlatStatBonus(8, 1, 5, 5);
        if (characterName == "Kanadé") opponent.applyFlatStatBonus(8, 2, 7, 8);
    }

    if (hasHazak && hasHestia && hasSanctus)
    {
        if (characterName == "Hazak") opponent.applyFlatStatBonus(18, 2, 5, 6);
        if (characterName == "Hestia") opponent.applyFlatStatBonus(24, 0, 3, 8);
        if (characterName == "Sanctus") opponent.applyFlatStatBonus(42, 0, 2, 2);
    }

    if (hasHazak && hasFail)
    {
        if (characterName == "Hazak") opponent.applyFlatStatBonus(6, 2, 4, 5);
        if (characterName == "Fail") opponent.applyFlatStatBonus(8, 2, 7, 8);
    }

    if (hasSkuro)
    {
        if (characterName == "Skuro") opponent.applyFlatStatBonus(-10, 4, 8, 10);
        else opponent.applyFlatStatBonus(4, 0, 2, 1);
    }
}

void AdventurerGroupEncounter::announceRelationshipBonus(
    const std::vector<std::string>& groupNames
)
{
    std::vector<std::string> lines;

    if (containsName(groupNames, "Hazak") && containsName(groupNames, "Hestia"))
    {
        lines.push_back("Relation active : Hazak protège Hestia.");
        lines.push_back("Ses attaques deviennent plus précises, mais le combat reste non-massacre.");
    }
    else if (containsName(groupNames, "Aoi") && containsName(groupNames, "Kanadé") && containsName(groupNames, "Sanctus"))
    {
        lines.push_back("Relation active : Sanctus tient la ligne pendant qu'Aoi et Kanadé préparent leurs sorts.");
    }
    else if (containsName(groupNames, "Mattzelda") && containsName(groupNames, "Louis") && containsName(groupNames, "Trexof"))
    {
        lines.push_back("Relation active : trio de potes.");
        lines.push_back("Tanking, improvisation et chaos amical se mélangent.");
    }
    else if (containsName(groupNames, "Skuro"))
    {
        lines.push_back("Relation active : Skuro rend le groupe plus dangereux, mais aussi plus instable.");
    }
    else if (containsName(groupNames, "Hestia") && containsName(groupNames, "Sanctus") && containsName(groupNames, "Hazak"))
    {
        lines.push_back("Relation active : protection sacrée.");
        lines.push_back("Sanctus tient la ligne, Hazak refuse le massacre, Hestia survit par réflexe divin.");
    }
    else if (containsName(groupNames, "Fail") && containsName(groupNames, "Aoi") && containsName(groupNames, "Kanadé"))
    {
        lines.push_back("Relation active : laboratoire vivant.");
        lines.push_back("Fail expérimente pendant qu'Aoi stabilise et que Kanadé surcharge les sorts.");
    }
    else if (containsName(groupNames, "Louis") && containsName(groupNames, "Fire Flight") && containsName(groupNames, "Trexof"))
    {
        lines.push_back("Relation active : pluie de projectiles.");
        lines.push_back("Louis veut aider, Fire Flight commande, Trexof teste les limites.");
    }
    else if (containsName(groupNames, "Hazak") && containsName(groupNames, "Henrique"))
    {
        lines.push_back("Relation active : Hazak et Henrique se connaissent trop bien pour laisser l'autre tomber facilement.");
    }
    else if (containsName(groupNames, "Hazak") && containsName(groupNames, "Fail"))
    {
        lines.push_back("Relation active : contrat de non-agression.");
        lines.push_back("Fail expérimente, Hazak surveille.");
    }
    else
    {
        return;
    }

    showEncounterScreen("RELATION DE GROUPE", "combat.encounter.adventurer_group.relationship", lines);
}

// EN: announceSpecialGroup declares or implements a focused behavior used by this module.
// FR: announceSpecialGroup déclare ou implémente un comportement précis utilisé par ce module.
void AdventurerGroupEncounter::announceSpecialGroup(const std::vector<std::string>& names)
{
    std::vector<std::string> lines;
    lines.push_back("Un groupe spécial répond à l'appel de l'arène.");
    lines.push_back("Ces groupes représentent environ 25% des tirages d'aventuriers.");

    for (const std::string& name : names)
    {
        lines.push_back("- " + name);
    }

    CombatIntent intent = SpecialEncounterRules::getIntentForSpecialGroup(names);
    lines.push_back(SpecialEncounterRules::getIntentText(intent, names));

    showEncounterScreen(
        "GROUPE SPÉCIAL",
        "combat.encounter.adventurer_group.special",
        lines
    );

    SpecialCharacterGroupDialogueCatalog::displayEntranceDialogue(names);

    for (const std::string& name : names)
    {
        if (SpecialCharacterDialogueCatalog::hasDialogueFor(name))
        {
            SpecialCharacterDialogueCatalog::displayEntranceDialogue(name);
        }
    }
}
