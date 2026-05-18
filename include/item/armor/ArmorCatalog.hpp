// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ITEM_ARMOR_ARMORCATALOG_HPP
#define INCLUDE_ITEM_ARMOR_ARMORCATALOG_HPP

#include "item/armor/Armor.hpp"

class ArmorCatalog
{
public:
    static Armor createSimpleOutfit();
    static Armor createWornLeatherArmor();
    static Armor createArenaChainmail();
};

#endif
