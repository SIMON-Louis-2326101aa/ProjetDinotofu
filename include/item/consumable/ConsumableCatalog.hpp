// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ITEM_CONSUMABLE_CONSUMABLECATALOG_HPP
#define INCLUDE_ITEM_CONSUMABLE_CONSUMABLECATALOG_HPP

#include "item/consumable/Consumable.hpp"

class ConsumableCatalog
{
public:
    static Consumable createBasicHealingPotion();
    static Consumable createBasicDamagePotion();

    static Consumable createReinforcedHealingPotion();
    static Consumable createReinforcedDamagePotion();
};

#endif
