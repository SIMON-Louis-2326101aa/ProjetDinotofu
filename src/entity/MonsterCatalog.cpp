// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "entity/MonsterCatalog.hpp"

#include <iostream>

Monster MonsterCatalog::createScaredGoblin()
{
    return Monster(
        "Gobelin peureux",
        "Assassin primitif",
        Race::Gobelin,
        1,
        55,
        8,
        14,
        18,
        0,
        0,
        false,
        false,
        false
    );
}

Monster MonsterCatalog::createBrutalGoblin()
{
    return Monster(
        "Gobelin brutal",
        "Bagarreur sauvage",
        Race::Gobelin,
        2,
        75,
        10,
        18,
        22,
        0,
        0,
        false,
        false,
        false
    );
}

Monster MonsterCatalog::createStarvingWolf()
{
    return Monster(
        "Loup affamé",
        "Prédateur rapide",
        Race::Bete,
        2,
        65,
        9,
        20,
        25,
        0,
        0,
        false,
        false,
        false
    );
}

Monster MonsterCatalog::createCrackedSkeleton()
{
    return Monster(
        "Squelette fissuré",
        "Mort-vivant fragile",
        Race::MortVivant,
        3,
        80,
        12,
        22,
        24,
        0,
        0,
        false,
        false,
        false
    );
}

Monster MonsterCatalog::createMinorOrc()
{
    return Monster(
        "Orc mineur",
        "Combattant lourd",
        Race::Orc,
        4,
        120,
        15,
        26,
        28,
        0,
        0,
        false,
        true,
        false
    );
}

Monster MonsterCatalog::createCaveBat()
{
    return Monster(
        "Chauve-souris des cavernes",
        "Créature rapide",
        Race::Bete,
        1,
        45,
        7,
        15,
        20,
        0,
        0,
        false,
        false,
        false
    );
}

Monster MonsterCatalog::createWildBoar()
{
    return Monster(
        "Sanglier sauvage",
        "Bête résistante",
        Race::Bete,
        3,
        95,
        12,
        21,
        24,
        0,
        0,
        false,
        false,
        false
    );
}

Monster MonsterCatalog::createLostBandit()
{
    return Monster(
        "Bandit perdu",
        "Humain opportuniste",
        Race::Humain,
        4,
        100,
        14,
        24,
        30,
        1,
        0,
        false,
        false,
        false
    );
}

Monster MonsterCatalog::createRandomMonsterForLevel(int level, Random& random)
{
    if (level <= 1)
    {
        int choice = random.between(1, 2);

        if (choice == 1)
        {
            return createScaredGoblin();
        }

        return createCaveBat();
    }

    if (level == 2)
    {
        int choice = random.between(1, 3);

        if (choice == 1)
        {
            return createBrutalGoblin();
        }

        if (choice == 2)
        {
            return createStarvingWolf();
        }

        return createCaveBat();
    }

    if (level == 3)
    {
        int choice = random.between(1, 3);

        if (choice == 1)
        {
            return createCrackedSkeleton();
        }

        if (choice == 2)
        {
            return createWildBoar();
        }

        return createBrutalGoblin();
    }

    int choice = random.between(1, 4);

    if (choice == 1)
    {
        return createMinorOrc();
    }

    if (choice == 2)
    {
        return createLostBandit();
    }

    if (choice == 3)
    {
        return createCrackedSkeleton();
    }

    return createWildBoar();
}

void MonsterCatalog::displayAvailableMonsters()
{
    std::cout << "========== MONSTRES DISPONIBLES ==========" << std::endl;
    std::cout << "1 : Gobelin peureux" << std::endl;
    std::cout << "2 : Gobelin brutal" << std::endl;
    std::cout << "3 : Loup affamé" << std::endl;
    std::cout << "4 : Squelette fissuré" << std::endl;
    std::cout << "5 : Orc mineur" << std::endl;
    std::cout << "6 : Chauve-souris des cavernes" << std::endl;
    std::cout << "7 : Sanglier sauvage" << std::endl;
    std::cout << "8 : Bandit perdu" << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << std::endl;
}
