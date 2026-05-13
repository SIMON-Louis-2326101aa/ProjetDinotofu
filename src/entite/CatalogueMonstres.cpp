#include "entite/CatalogueMonstres.hpp"

#include <iostream>

Monstre CatalogueMonstres::creerGobelin()
{
    return Monstre(
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

Monstre CatalogueMonstres::creerGobelinBrutal()
{
    return Monstre(
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

Monstre CatalogueMonstres::creerLoupAffame()
{
    return Monstre(
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

Monstre CatalogueMonstres::creerSquelette()
{
    return Monstre(
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

Monstre CatalogueMonstres::creerOrcMineur()
{
    return Monstre(
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

void CatalogueMonstres::afficherMonstresDisponibles()
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