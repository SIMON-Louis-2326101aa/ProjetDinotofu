// English: This file prepares random human and semi-human AI opponents for Dinotofu.
// Français : Ce fichier prépare les adversaires IA humains et semi-humains aléatoires de Dinotofu.

#include "character/RandomCharacterGenerator.hpp"

#include "character/RandomNameGenerator.hpp"
#include "character/SpecialCharacterCatalog.hpp"
#include "character/SpecialCharacterNativeBonus.hpp"
#include "class_system/ClassCatalog.hpp"
#include "progression/DifficultyMode.hpp"

CharacterRace RandomCharacterGenerator::generateRace(Random& random)
{
    int roll = random.between(1, 100);

    if (roll <= 35)
    {
        return CharacterRace::Human;
    }

    if (roll <= 47)
    {
        return CharacterRace::Elf;
    }

    if (roll <= 58)
    {
        return CharacterRace::DarkElf;
    }

    if (roll <= 66)
    {
        return CharacterRace::Dwarf;
    }

    if (roll <= 72)
    {
        return CharacterRace::Gnome;
    }

    if (roll <= 77)
    {
        return CharacterRace::Halfling;
    }

    if (roll <= 83)
    {
        return CharacterRace::Tiefling;
    }

    if (roll <= 88)
    {
        return CharacterRace::Aasimar;
    }

    if (roll <= 92)
    {
        return CharacterRace::Kitsune;
    }

    if (roll <= 95)
    {
        return CharacterRace::Fairy;
    }

    if (roll <= 97)
    {
        return CharacterRace::Orc;
    }

    if (roll <= 99)
    {
        return CharacterRace::Vampire;
    }

    return CharacterRace::HalfDragon;
}

Player RandomCharacterGenerator::generateClassicOpponent(Random& random)
{
    int classChoice = random.between(1, ClassCatalog::getPlayableClassCount());
    PlayerClass randomClass = ClassCatalog::createBaseClass(classChoice);

    return generateClassicOpponentWithClass(
        randomClass,
        random
    );
}

Player RandomCharacterGenerator::generateClassicOpponentWithClass(
    const PlayerClass& playerClass,
    Random& random
)
{
    CharacterRace race = generateRace(random);
    std::string name = RandomNameGenerator::generateNameForRace(race, random);

    Player opponent(name, playerClass);
    opponent.setRace(race);
    opponent.initializeStarterInventory(DifficultyMode::Normal);

    return opponent;
}

Player RandomCharacterGenerator::generateSpecialOpponent(Random& random)
{
    SpecialCharacter specialCharacter = SpecialCharacterCatalog::createRandomSpecialOpponent(random);

    Player opponent(
        specialCharacter.getName(),
        ClassCatalog::createClassByName(specialCharacter.getNativeClass())
    );

    opponent.setRace(specialCharacter.getRace());
    opponent.initializeStarterInventory(DifficultyMode::Normal);

    SpecialCharacterNativeBonus::applyForSpecialCharacter(
        opponent,
        specialCharacter
    );

    return opponent;
}

Player RandomCharacterGenerator::generateArenaOpponent(Random& random)
{
    int roll = random.between(1, 100);

    if (roll <= SPECIAL_ARENA_SPAWN_PERCENTAGE)
    {
        return generateSpecialOpponent(random);
    }

    return generateClassicOpponent(random);
}

Player RandomCharacterGenerator::generateRandomOpponent(Random& random)
{
    return generateClassicOpponent(random);
}
