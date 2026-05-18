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

void MonsterCatalog::displayAvailableMonsters()
{
    std::cout << "========== MONSTRES DISPONIBLES ==========" << std::endl;
    std::cout << "1 : Gobelin peureux" << std::endl;
    std::cout << "2 : Gobelin brutal" << std::endl;
    std::cout << "3 : Loup affamé" << std::endl;
    std::cout << "4 : Squelette fissuré" << std::endl;
    std::cout << "5 : Orc mineur" << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << std::endl;
}