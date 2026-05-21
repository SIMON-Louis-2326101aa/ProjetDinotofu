// EN: ArmorCatalog.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: ArmorCatalog.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ITEM_ARMOR_ARMORCATALOG_HPP
#define INCLUDE_ITEM_ARMOR_ARMORCATALOG_HPP

#include "item/armor/Armor.hpp"

class ArmorCatalog
{
public:
    // EN: createSimpleOutfit declares or implements a focused behavior used by this module.
    // FR: createSimpleOutfit déclare ou implémente un comportement précis utilisé par ce module.
    static Armor createSimpleOutfit();
    // EN: createWornLeatherArmor declares or implements a focused behavior used by this module.
    // FR: createWornLeatherArmor déclare ou implémente un comportement précis utilisé par ce module.
    static Armor createWornLeatherArmor();
    // EN: createArenaChainmail declares or implements a focused behavior used by this module.
    // FR: createArenaChainmail déclare ou implémente un comportement précis utilisé par ce module.
    static Armor createArenaChainmail();
};

#endif
