// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/weapon/WeaponCatalog.hpp"

Weapon WeaponCatalog::createBareHands()
{
    return Weapon(
        "Mains nues",
        "Aucune arme équipée. Il va falloir avoir du courage.",
        0,
        WeaponType::BareHands,
        0,
        0,
        0,
        -1
    );
}

Weapon WeaponCatalog::createRustySword()
{
    return Weapon(
        "Épée rouillée",
        "Une vieille lame abîmée, mais toujours capable de faire regretter un mauvais placement.",
        15,
        WeaponType::Sword,
        1,
        3,
        2,
        80
    );
}

Weapon WeaponCatalog::createArenaBlade()
{
    return Weapon(
        "Lame d'arène",
        "Une arme simple, forgée pour les duels rapides et les victoires propres.",
        40,
        WeaponType::Sword,
        3,
        6,
        5,
        120
    );
}