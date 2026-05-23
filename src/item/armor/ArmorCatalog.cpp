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
        || normalized.find("clerc") != std::string::npos)
    {
        return createApprenticeRobe();
    }

    if (normalized.find("colosse") != std::string::npos
        || normalized.find("tank") != std::string::npos
        || normalized.find("gardien") != std::string::npos
        || normalized.find("paladin") != std::string::npos
        || normalized.find("templier") != std::string::npos
        || normalized.find("briseur") != std::string::npos)
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
