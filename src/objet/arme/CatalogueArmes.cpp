#include "objet/arme/CatalogueArmes.hpp"

Arme CatalogueArmes::creerMainsNues()
{
    return Arme(
        "Mains nues",
        "Aucune arme équipée. Il va falloir avoir du courage.",
        0,
        TypeArme::MainsNues,
        0,
        0,
        0,
        -1
    );
}

Arme CatalogueArmes::creerEpeeRouillee()
{
    return Arme(
        "Épée rouillée",
        "Une vieille lame abîmée, mais toujours capable de faire regretter un mauvais placement.",
        15,
        TypeArme::Epee,
        1,
        3,
        2,
        80
    );
}

Arme CatalogueArmes::creerLameArene()
{
    return Arme(
        "Lame d'arène",
        "Une arme simple, forgée pour les duels rapides et les victoires propres.",
        40,
        TypeArme::Epee,
        3,
        6,
        5,
        120
    );
}