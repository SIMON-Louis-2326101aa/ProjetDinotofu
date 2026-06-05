// EN: ArmorCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: ArmorCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/armor/ArmorCatalog.hpp"

#include <algorithm>
#include <cctype>
#include <string>

// EN: createSimpleOutfit declares or implements a focused behavior used by this module.
// FR: createSimpleOutfit déclare ou implémente un comportement précis utilisé par ce module.
Armor ArmorCatalog::createSimpleOutfit()
{
    return Armor(
        "Tenue simple",
        "Une tenue légère, sans vraie protection, mais assez confortable pour courir avant de regretter ses choix.",
        0,
        ArmorType::Cloth,
        0,
        0,
        -1
    );
}

// EN: createWornLeatherArmor declares or implements a focused behavior used by this module.
// FR: createWornLeatherArmor déclare ou implémente un comportement précis utilisé par ce module.
Armor ArmorCatalog::createWornLeatherArmor()
{
    return Armor(
        "Armure en cuir usée",
        "Une protection fatiguée, mais capable d'encaisser les premières erreurs d'un combattant trop confiant.",
        25,
        ArmorType::Leather,
        20,
        2,
        90
    );
}


Armor ArmorCatalog::createApprenticeRobe()
{
    return Armor(
        "Robe d'apprenti renforcée",
        "Une robe légère avec quelques coutures renforcées. Elle protège peu, mais ne gêne pas les gestes magiques.",
        18,
        ArmorType::Cloth,
        8,
        1,
        80
    );
}

Armor ArmorCatalog::createPaddedVest()
{
    return Armor(
        "Veste matelassée de départ",
        "Une protection souple pour survivre aux premières erreurs sans transformer le porteur en boîte de conserve.",
        30,
        ArmorType::Leather,
        14,
        2,
        85
    );
}

Armor ArmorCatalog::createHeavyPaddedArmor()
{
    return Armor(
        "Armure lourde rafistolée",
        "Une armure de départ lourde et imparfaite. Elle encaisse mieux, mais elle n'a rien d'une relique.",
        42,
        ArmorType::Chainmail,
        24,
        3,
        90
    );
}

namespace
{
    std::string normalizeArmorClassName(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return value;
    }
}

Armor ArmorCatalog::createReinforcedLeatherArmor()
{
    return Armor(
        "Armure en cuir renforcée",
        "Un cuir mieux entretenu, avec quelques plaques souples. Idéal pour progresser sans perdre toute mobilité.",
        115,
        ArmorType::Leather,
        24,
        3,
        115
    );
}

Armor ArmorCatalog::createGuardChainmail()
{
    return Armor(
        "Cotte de maille de garde",
        "Une maille solide de milice, plus lourde que le cuir mais rassurante quand la route devient mauvaise.",
        155,
        ArmorType::Chainmail,
        34,
        5,
        135
    );
}

Armor ArmorCatalog::createRunedApprenticeRobe()
{
    return Armor(
        "Robe runique d'apprenti",
        "Une robe de mage renforcée par des coutures runiques simples. Elle protège mieux sans casser la posture magique.",
        130,
        ArmorType::Magical,
        18,
        3,
        105
    );
}

Armor ArmorCatalog::createCrudePlateArmor()
{
    return Armor(
        "Armure de plaques grossière",
        "De grosses plaques imparfaites, lourdes et bruyantes, mais capables de sauver un colosse trop confiant.",
        190,
        ArmorType::Plate,
        48,
        7,
        150
    );
}

Armor ArmorCatalog::createTravelerScaleVest()
{
    return Armor(
        "Veste d'écailles de voyage",
        "Une veste souple renforcée de petites plaques. Elle protège mieux que le cuir sans voler toute la mobilité.",
        165,
        ArmorType::Leather,
        29,
        4,
        125
    );
}

Armor ArmorCatalog::createThreadedRuneRobe()
{
    return Armor(
        "Robe à fils runiques",
        "Une robe de mage plus stable, cousue pour encaisser les petites erreurs de canalisation et les coups perdus.",
        170,
        ArmorType::Magical,
        21,
        4,
        112
    );
}

Armor ArmorCatalog::createMilitiaHalfPlate()
{
    return Armor(
        "Demi-plaques de milice",
        "Un compromis lourd mais encore portable : moins noble qu'une vraie armure de plaques, plus sûr qu'une simple maille.",
        230,
        ArmorType::Plate,
        42,
        6,
        145
    );
}

Armor ArmorCatalog::createRunicChainmail()
{
    return Armor(
        "Cotte runique de garde",
        "Cotte de mailles renforcée : palier intermédiaire, bonne défense physique et rune simple contre les chocs faibles.",
        380,
        ArmorType::Chainmail,
        48,
        7,
        165
    );
}

Armor ArmorCatalog::createShadowThreadCoat()
{
    return Armor(
        "Manteau cousu d'ombre",
        "Tenue rare légère : discrétion, résistance magique correcte et faiblesse assumée face aux coups lourds.",
        330,
        ArmorType::Leather,
        32,
        5,
        128
    );
}

Armor ArmorCatalog::createPolishedScaleHarness()
{
    return Armor(
        "Harnais d'écailles polies",
        "Armure semi-lourde issue de matériaux de monstre : protège bien, mais demande plus d'entretien.",
        430,
        ArmorType::Chainmail,
        52,
        8,
        170
    );
}

Armor ArmorCatalog::createDamagedCartographerCoat()
{
    return Armor("Manteau de cartographe abîmé", "Manteau léger couvert de poches, cartes et coutures rapides. Protège peu, mais reste parfait pour explorer sans bruit.", 210, ArmorType::Leather, 28, 4, 118);
}

Armor ArmorCatalog::createSunDriedClayBreastplate()
{
    return Armor("Plastron d'argile séchée", "Protection semi-rigide renforcée à l'argile rouge. Très correcte contre les chocs secs, moins rassurante sous la pluie.", 280, ArmorType::Plate, 44, 6, 142);
}

Armor ArmorCatalog::createLivingFiberRobe()
{
    return Armor("Robe aux fibres vivantes", "Robe magique cousue avec des fibres végétales encore réactives. Elle absorbe mieux les petites erreurs de canalisation.", 310, ArmorType::Magical, 25, 5, 118);
}

Armor ArmorCatalog::createWhistlingMinerHarness()
{
    return Armor("Harnais de mineur sifflant", "Harnais lourd équipé de plaques froides et ressorts simples. Bruyant, solide, presque trop rassurant.", 355, ArmorType::Chainmail, 50, 7, 158);
}


Armor ArmorCatalog::createDrownedArchivistVest()
{
    return Armor("Gilet d'archiviste noyé", "Gilet couvert de poches sèches par miracle. Il protège surtout les côtes, les papiers importants et l'ego des scribes.", 325, ArmorType::Leather, 36, 5, 134);
}

Armor ArmorCatalog::createGreyDrakeHarness()
{
    return Armor("Harnais de drake gris", "Harnais semi-lourd renforcé par des écailles râpeuses. Solide pour les falaises, un peu pénible à entretenir.", 460, ArmorType::Chainmail, 56, 8, 178);
}

Armor ArmorCatalog::createPatchworkCarnivalCape()
{
    return Armor("Cape de foire décousue", "Cape légère cousue de tissus criards et de perles miroir. Défense modeste, mais bonne pour survivre au premier mauvais regard.", 290, ArmorType::Leather, 30, 4, 122);
}


Armor ArmorCatalog::createColdSurvivalParka()
{
    return Armor("Parka de survie glaciale", "Tenue épaisse pensée pour les biomes froids. Elle prend la place d'une vraie armure : excellente contre le froid, moyenne contre les coups.", 260, ArmorType::Leather, 24, 3, 120);
}

Armor ArmorCatalog::createHeatSurvivalSuit()
{
    return Armor("Tenue ignifugée de terrain", "Tenue traitée contre chaleur, braises et projections faibles. Elle remplace une armure classique et protège moins bien physiquement.", 290, ArmorType::Leather, 22, 3, 115);
}

Armor ArmorCatalog::createInsulatedExplorerCoat()
{
    return Armor("Manteau isolant d'explorateur", "Manteau utilitaire contre les températures pénibles. Moins spécialisé qu'une parka ou une tenue ignifugée, mais très pratique en expédition.", 340, ArmorType::Leather, 30, 4, 130);
}

Armor ArmorCatalog::createStarterArmorForClass(const std::string& className)
{
    std::string normalized = normalizeArmorClassName(className);

    if (normalized.find("mage") != std::string::npos
        || normalized.find("sorc") != std::string::npos
        || normalized.find("arcan") != std::string::npos
        || normalized.find("alchim") != std::string::npos
        || normalized.find("invoc") != std::string::npos
        || normalized.find("nécro") != std::string::npos
        || normalized.find("necro") != std::string::npos
        || normalized.find("prêtre") != std::string::npos
        || normalized.find("pretre") != std::string::npos
        || normalized.find("clerc") != std::string::npos
        || normalized.find("mancien") != std::string::npos
        || normalized.find("démoniste") != std::string::npos
        || normalized.find("demoniste") != std::string::npos
        || normalized.find("runiste") != std::string::npos
        || normalized.find("enchanteur") != std::string::npos)
    {
        return createApprenticeRobe();
    }

    if (normalized.find("colosse") != std::string::npos
        || normalized.find("tank") != std::string::npos
        || normalized.find("gardien") != std::string::npos
        || normalized.find("paladin") != std::string::npos
        || normalized.find("templier") != std::string::npos
        || normalized.find("briseur") != std::string::npos
        || normalized.find("bouclier") != std::string::npos
        || normalized.find("porte-bannière") != std::string::npos
        || normalized.find("porte-banniere") != std::string::npos
        || normalized.find("géomancien") != std::string::npos
        || normalized.find("geomancien") != std::string::npos
        || normalized.find("lame tellurique") != std::string::npos)
    {
        return createHeavyPaddedArmor();
    }

    return createPaddedVest();
}

// EN: createArenaChainmail declares or implements a focused behavior used by this module.
// FR: createArenaChainmail déclare ou implémente un comportement précis utilisé par ce module.
Armor ArmorCatalog::createArenaChainmail()
{
    return Armor(
        "Cotte de maille d'arène",
        "Une armure équilibrée, forgée pour les duels où chaque coup laisse une trace.",
        75,
        ArmorType::Chainmail,
        45,
        4,
        140
    );
}
