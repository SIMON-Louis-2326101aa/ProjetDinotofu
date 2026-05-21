// EN: ArmorCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: ArmorCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/armor/ArmorCatalog.hpp"

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
