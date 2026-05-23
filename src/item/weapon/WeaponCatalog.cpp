// EN: WeaponCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: WeaponCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/weapon/WeaponCatalog.hpp"

#include <algorithm>
#include <cctype>
#include <string>

// EN: createBareHands declares or implements a focused behavior used by this module.
// FR: createBareHands déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: createRustySword declares or implements a focused behavior used by this module.
// FR: createRustySword déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: createArenaBlade declares or implements a focused behavior used by this module.
// FR: createArenaBlade déclare ou implémente un comportement précis utilisé par ce module.
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

namespace
{
    std::string normalizeWeaponClassName(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return value;
    }
}

Weapon WeaponCatalog::createTrainingDagger()
{
    return Weapon("Dague d'entraînement", "Une lame courte, logique pour un débutant discret.", 16, WeaponType::Dagger, 2, 5, 4, 75);
}

Weapon WeaponCatalog::createTrainingSpear()
{
    return Weapon("Lance d'entraînement", "Une lance simple, plus cohérente pour apprendre à garder la distance.", 18, WeaponType::Spear, 2, 6, 3, 85);
}

Weapon WeaponCatalog::createTrainingBow()
{
    return Weapon("Arc d'entraînement", "Un arc basique, fragile, mais fait pour la distance.", 18, WeaponType::Bow, 2, 5, 4, 80);
}

Weapon WeaponCatalog::createTrainingCrossbow()
{
    return Weapon("Arbalète d'entraînement", "Une arbalète simple : lente, mais cohérente avec les carreaux.", 20, WeaponType::Bow, 3, 6, 3, 85);
}

Weapon WeaponCatalog::createTrainingThrowingBandolier()
{
    return Weapon("Bandoulière de lancer", "Un équipement de jet basique pour utiliser des couteaux de lancer à courte distance.", 17, WeaponType::Bow, 2, 5, 5, 70);
}

Weapon WeaponCatalog::createTrainingStaff()
{
    return Weapon("Bâton d'apprenti", "Un bâton simple, utile aux mages et aux voyageurs prudents.", 15, WeaponType::Staff, 1, 4, 3, 80);
}

Weapon WeaponCatalog::createHeavyTrainingAxe()
{
    return Weapon("Hache lourde émoussée", "Une arme lourde de débutant : dangereuse, mais pas encore héroïque.", 22, WeaponType::Axe, 2, 7, 3, 90);
}

Weapon WeaponCatalog::createEmergencyWoodKnife()
{
    return Weapon(
        "Couteau de bois d\'urgence",
        "Un petit couteau en bois presque nul. Ce n\'est pas fait pour gagner un duel, juste pour ne pas être totalement sans défense au corps à corps.",
        3,
        WeaponType::Dagger,
        0,
        1,
        0,
        35
    );
}

Weapon WeaponCatalog::createStarterWeaponForClass(const std::string& className)
{
    std::string normalized = normalizeWeaponClassName(className);

    if (normalized.find("lancier") != std::string::npos)
    {
        return createTrainingSpear();
    }

    if (normalized.find("lanceur de dagues") != std::string::npos)
    {
        return createTrainingThrowingBandolier();
    }

    if (normalized.find("assassin") != std::string::npos
        || normalized.find("dagues") != std::string::npos
        || normalized.find("duelliste") != std::string::npos
        || normalized.find("ombre") != std::string::npos)
    {
        return createTrainingDagger();
    }

    if (normalized.find("arbal") != std::string::npos)
    {
        return createTrainingCrossbow();
    }

    if (normalized.find("archer") != std::string::npos
        || normalized.find("rôdeur") != std::string::npos
        || normalized.find("rodeur") != std::string::npos
        || normalized.find("chasseur") != std::string::npos
        || normalized.find("tireur") != std::string::npos
        || normalized.find("artificier") != std::string::npos)
    {
        return createTrainingBow();
    }

    if (normalized.find("mage") != std::string::npos
        || normalized.find("sorc") != std::string::npos
        || normalized.find("arcan") != std::string::npos
        || normalized.find("alchim") != std::string::npos
        || normalized.find("prêtre") != std::string::npos
        || normalized.find("pretre") != std::string::npos
        || normalized.find("clerc") != std::string::npos)
    {
        return createTrainingStaff();
    }

    if (normalized.find("colosse") != std::string::npos
        || normalized.find("barbare") != std::string::npos
        || normalized.find("briseur") != std::string::npos
        || normalized.find("berserker") != std::string::npos)
    {
        return createHeavyTrainingAxe();
    }

    return createRustySword();
}
