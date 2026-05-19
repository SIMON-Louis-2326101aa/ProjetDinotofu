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
#include "core/Console.hpp"
#include "entity/Race.hpp"
#include "progression/DifficultyMode.hpp"

#include <algorithm>
#include <iostream>

namespace
{
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

    int scaleMaxHp(int baseMaxHp, int encounterLevel)
    {
        return baseMaxHp + ((encounterLevel - 1) * 12);
    }

    int scaleMinDamage(int baseMinDamage, int encounterLevel)
    {
        return baseMinDamage + ((encounterLevel - 1) / 2);
    }

    int scaleMaxDamage(int baseMaxDamage, int encounterLevel)
    {
        return baseMaxDamage + (encounterLevel - 1);
    }

    int scaleCriticalDamage(int baseCriticalDamage, int encounterLevel)
    {
        return baseCriticalDamage + ((encounterLevel - 1) * 2);
    }

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

void AdventurerGroupEncounter::displayGroupEncounterIntroduction()
{
    Console::clear();

    std::cout << "Tu ne tombes pas sur une vague de monstres." << std::endl;
    std::cout << "Cette fois, l'arène appelle un groupe d'aventuriers." << std::endl;
    std::cout << "Ils ne sont pas forcément là pour te tuer, mais ils ne sont pas venus pour applaudir non plus." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(2);
}

EnemyCombatQueue AdventurerGroupEncounter::createClassicRandomGroup(
    const Player& player,
    Random& random
)
{
    EnemyCombatQueue queue;
    int groupSize = random.between(2, 3);
    int encounterLevel = player.getLevel();

    std::cout << "Un groupe d'aventuriers inconnus entre dans l'arène." << std::endl;
    std::cout << "Noms, races et classes sont générés par l'arène. Aucun personnage spécial ici." << std::endl;
    std::cout << std::endl;

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
    int groupChoice = random.between(1, 8);
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
        default:
            names = {"Fire Flight", "Matt (PRO)"};
            break;
    }

    announceSpecialGroup(names);

    for (const std::string& name : names)
    {
        addSpecialCharacterAsOpponent(queue, name, encounterLevel);
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
    int encounterLevel
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

    addPlayerAsOpponent(queue, opponent, encounterLevel);
}

void AdventurerGroupEncounter::announceSpecialGroup(const std::vector<std::string>& names)
{
    std::cout << "Un groupe spécial répond à l'appel de l'arène." << std::endl;
    std::cout << "Ces groupes représentent environ 25% des tirages d'aventuriers." << std::endl;

    for (const std::string& name : names)
    {
        std::cout << "- " << name << std::endl;
    }

    std::cout << std::endl;
    CombatIntent intent = SpecialEncounterRules::getIntentForSpecialGroup(names);
    std::cout << SpecialEncounterRules::getIntentText(intent, names) << std::endl;
    std::cout << std::endl;

    SpecialCharacterGroupDialogueCatalog::displayEntranceDialogue(names);

    for (const std::string& name : names)
    {
        if (SpecialCharacterDialogueCatalog::hasDialogueFor(name))
        {
            SpecialCharacterDialogueCatalog::displayEntranceDialogue(name);
        }
    }

    Console::pauseSeconds(3);
}
