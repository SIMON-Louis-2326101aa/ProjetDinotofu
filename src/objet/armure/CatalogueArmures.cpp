#include "objet/armure/CatalogueArmures.hpp"

Armure CatalogueArmures::creerTenueSimple()
{
    return Armure(
        "Tenue simple",
        "Une tenue légère, sans vraie protection, mais assez confortable pour courir avant de regretter ses choix.",
        0,
        TypeArmure::Tissu,
        0,
        0,
        -1
    );
}

Armure CatalogueArmures::creerArmureCuirUsee()
{
    return Armure(
        "Armure en cuir usée",
        "Une protection fatiguée, mais capable d'encaisser les premières erreurs d'un combattant trop confiant.",
        25,
        TypeArmure::Cuir,
        20,
        2,
        90
    );
}

Armure CatalogueArmures::creerCotteMailleArene()
{
    return Armure(
        "Cotte de maille d'arène",
        "Une armure équilibrée, forgée pour les duels où chaque coup laisse une trace.",
        75,
        TypeArmure::Maille,
        45,
        4,
        140
    );
}