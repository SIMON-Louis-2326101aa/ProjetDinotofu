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


Weapon WeaponCatalog::createIronSword()
{
    return Weapon("Épée de fer simple", "Une lame de fer honnête : pas héroïque, mais déjà plus sérieuse que l'équipement de départ.", 135, WeaponType::Sword, 4, 9, 6, 135);
}

Weapon WeaponCatalog::createReinforcedDagger()
{
    return Weapon("Dague renforcée", "Une dague plus propre, pensée pour les profils rapides qui veulent éviter les armes rouillées.", 120, WeaponType::Dagger, 4, 8, 8, 120);
}

Weapon WeaponCatalog::createGuardSpear()
{
    return Weapon("Lance de garde", "Une lance robuste de milice, correcte pour garder l'ennemi à distance en début de progression.", 145, WeaponType::Spear, 4, 10, 5, 140);
}

Weapon WeaponCatalog::createHuntingBow()
{
    return Weapon("Arc de chasse", "Un arc plus fiable que l'arc d'entraînement, utile dès que les munitions suivent.", 150, WeaponType::Bow, 4, 9, 7, 125);
}

Weapon WeaponCatalog::createApprenticeStaff()
{
    return Weapon("Bâton canalisateur", "Un bâton renforcé avec une encoche arcanique simple, prévu pour les mages prudents.", 130, WeaponType::Staff, 3, 8, 6, 130);
}

Weapon WeaponCatalog::createHeavyIronAxe()
{
    return Weapon("Hache lourde de fer", "Une hache massive, lente et chère à réparer, mais bien plus menaçante qu'une hache d'entraînement.", 170, WeaponType::Axe, 5, 12, 5, 150);
}

Weapon WeaponCatalog::createWorkshopHammer()
{
    return Weapon("Marteau d'atelier renforcé", "Un marteau de travail détourné pour le combat : lourd, fiable, sans être une arme de légende.", 160, WeaponType::Hammer, 4, 11, 4, 140);
}

Weapon WeaponCatalog::createPatrolCrossbow()
{
    return Weapon("Arbalète de patrouille", "Une arbalète plus régulière que les modèles d'entraînement, pensée pour tenir une route dangereuse.", 165, WeaponType::Bow, 5, 10, 4, 135);
}

Weapon WeaponCatalog::createBalancedRapier()
{
    return Weapon("Rapière équilibrée", "Une lame fine, correcte pour un dueliste qui préfère la précision à la force brute.", 155, WeaponType::Sword, 5, 9, 9, 120);
}

Weapon WeaponCatalog::createMercenarySabre()
{
    return Weapon("Sabre de mercenaire", "Une lame courbe de route, assez fiable pour les contrats dangereux sans entrer dans le matériel héroïque.", 210, WeaponType::Sword, 6, 11, 7, 145);
}

Weapon WeaponCatalog::createCurvedAmbushDagger()
{
    return Weapon("Dague courbe d'embuscade", "Une dague courte et nerveuse, pensée pour les combattants rapides qui veulent frapper proprement puis disparaître.", 185, WeaponType::Dagger, 5, 10, 10, 115);
}

Weapon WeaponCatalog::createMilitiaLongbow()
{
    return Weapon("Arc long de milice", "Un arc plus tendu que l'arc de chasse, utilisé pour tenir une ligne ou couvrir une retraite.", 215, WeaponType::Bow, 5, 11, 8, 120);
}

Weapon WeaponCatalog::createBoundOakStaff()
{
    return Weapon("Bâton de chêne lié", "Un bâton de canalisation bas-moyen, renforcé par des liens simples pour mieux supporter les sorts répétés.", 205, WeaponType::Staff, 4, 10, 8, 135);
}

Weapon WeaponCatalog::createRunicIronBlade()
{
    return Weapon("Lame de fer runique", "Arme de palier intermédiaire : fer renforcé, rainure runique et effet latent de brise-garde léger.", 360, WeaponType::Sword, 7, 14, 7, 165);
}

Weapon WeaponCatalog::createAmberEdgeDagger()
{
    return Weapon("Dague d'ambre vive", "Dague rapide préparée pour assassins : effet latent de saignement faible si la cible est déjà fragilisée.", 310, WeaponType::Dagger, 6, 12, 12, 145);
}

Weapon WeaponCatalog::createAshenLongbow()
{
    return Weapon("Arc long cendré", "Arc de palier avancé léger : flèches plus stables, meilleure portée et amorce de brûlure sur munitions adaptées.", 340, WeaponType::Bow, 6, 13, 9, 150);
}

Weapon WeaponCatalog::createChannelingScepter()
{
    return Weapon("Sceptre canalisateur", "Catalyseur simple pour mages : réduit les pertes de stabilité des petits sorts tant qu'il reste en bon état.", 390, WeaponType::Staff, 5, 14, 11, 142);
}

Weapon WeaponCatalog::createRelayFalchion()
{
    return Weapon("Fauchon de relais", "Lame de route courte et solide, forgée pour les gardes de relais qui doivent frapper vite dans les couloirs étroits.", 275, WeaponType::Sword, 6, 12, 8, 148);
}

Weapon WeaponCatalog::createWhistlingMineHammer()
{
    return Weapon("Marteau de mine sifflante", "Marteau lourd dont la tête vibre légèrement. Excellent contre les constructions, mais cher à réparer.", 420, WeaponType::Hammer, 8, 17, 6, 175);
}

Weapon WeaponCatalog::createSingingResinStaff()
{
    return Weapon("Bâton de résine chantante", "Catalyseur végétal dont la résine répond aux petits sorts. Stable pour mages, étrange pour les autres.", 405, WeaponType::Staff, 5, 15, 12, 145);
}

Weapon WeaponCatalog::createColdLanternBow()
{
    return Weapon("Arc des lanternes froides", "Arc souple renforcé avec résine d'écho et fibres froides. Les tirs semblent mieux tenir les distances nocturnes.", 375, WeaponType::Bow, 7, 14, 10, 138);
}

Weapon WeaponCatalog::createRedClaySabre()
{
    return Weapon("Sabre d'argile rouge", "Sabre protégé par une couche d'argile cuite. Moins noble qu'une lame runique, mais très fiable en terrain sec.", 330, WeaponType::Sword, 7, 13, 7, 160);
}

Weapon WeaponCatalog::createBrokenMapDagger()
{
    return Weapon("Dague de carte brisée", "Dague fine gravée à partir d'un fragment cartographique. Parfaite pour les éclaireurs qui frappent et changent de route.", 315, WeaponType::Dagger, 6, 12, 13, 120);
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

    if (normalized.find("lancier") != std::string::npos
        || normalized.find("javelinier") != std::string::npos)
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
        || normalized.find("sabreur") != std::string::npos
        || normalized.find("danseur") != std::string::npos
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
        || normalized.find("trappeur") != std::string::npos
        || normalized.find("guetteur") != std::string::npos
        || normalized.find("messager arm") != std::string::npos
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
        || normalized.find("clerc") != std::string::npos
        || normalized.find("mancien") != std::string::npos
        || normalized.find("démoniste") != std::string::npos
        || normalized.find("demoniste") != std::string::npos
        || normalized.find("runiste") != std::string::npos
        || normalized.find("enchanteur") != std::string::npos)
    {
        return createTrainingStaff();
    }

    if (normalized.find("colosse") != std::string::npos
        || normalized.find("barbare") != std::string::npos
        || normalized.find("briseur") != std::string::npos
        || normalized.find("berserker") != std::string::npos
        || normalized.find("faucheur") != std::string::npos
        || normalized.find("siège") != std::string::npos
        || normalized.find("siege") != std::string::npos)
    {
        return createHeavyTrainingAxe();
    }

    return createRustySword();
}


Weapon WeaponCatalog::createFireflyIronRapier()
{
    return Weapon("Rapière des lucioles de fer", "Lame fine montée avec des carapaces lumineuses. Rapide, précise, parfaite pour ceux qui veulent voir l'erreur arriver avant de la faire.", 455, WeaponType::Sword, 11, 20, 10, 142);
}

Weapon WeaponCatalog::createDrownedLedgerMace()
{
    return Weapon("Masse du registre noyé", "Masse lourde lestée avec un noyau d'engrenage et des pages durcies. Très administrative : elle tamponne directement sur l'armure.", 510, WeaponType::Hammer, 16, 27, 3, 165);
}

Weapon WeaponCatalog::createGreyCliffSpear()
{
    return Weapon("Lance des falaises grises", "Lance renforcée à l'écaille de drake gris. Bonne portée, bonne stabilité, mauvaise idée si tu as le vertige.", 485, WeaponType::Spear, 13, 24, 7, 152);
}

Weapon WeaponCatalog::createBrokenCarnivalWhip()
{
    return Weapon("Fouet de foire cassée", "Arme souple bricolée avec tickets, perles miroir et mauvaise ambiance. Elle ne frappe pas toujours fort, mais elle gêne très bien.", 430, WeaponType::Dagger, 9, 18, 12, 126);
}
