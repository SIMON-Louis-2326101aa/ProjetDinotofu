// EN: ConsumableCatalog.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: ConsumableCatalog.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ITEM_CONSUMABLE_CONSUMABLECATALOG_HPP
#define INCLUDE_ITEM_CONSUMABLE_CONSUMABLECATALOG_HPP

#include "item/consumable/Consumable.hpp"

class ConsumableCatalog
{
public:
    // EN: createBasicHealingPotion declares or implements a focused behavior used by this module.
    // FR: createBasicHealingPotion déclare ou implémente un comportement précis utilisé par ce module.
    static Consumable createBasicHealingPotion();
    // EN: createBasicDamagePotion declares or implements a focused behavior used by this module.
    // FR: createBasicDamagePotion déclare ou implémente un comportement précis utilisé par ce module.
    static Consumable createBasicDamagePotion();

    // EN: createReinforcedHealingPotion declares or implements a focused behavior used by this module.
    // FR: createReinforcedHealingPotion déclare ou implémente un comportement précis utilisé par ce module.
    static Consumable createMinorHealingPotion();
    static Consumable createReinforcedHealingPotion();
    static Consumable createGreaterHealingPotion();
    static Consumable createMajorHealingPotion();
    // EN: createReinforcedDamagePotion declares or implements a focused behavior used by this module.
    // FR: createReinforcedDamagePotion déclare ou implémente un comportement précis utilisé par ce module.
    static Consumable createMinorDamagePotion();
    static Consumable createReinforcedDamagePotion();
    static Consumable createGreaterDamagePotion();
    static Consumable createExperimentalDamagePotion();
    static Consumable createDefensivePotion();
    static Consumable createGreaterDefensivePotion();
    static Consumable createPrecisionPotion();
    static Consumable createWeakeningDebuffPotion();
    static Consumable createAntidotePotion();
    static Consumable createBurnSalvePotion();
    static Consumable createFrostResistancePotion();
    static Consumable createShockResistancePotion();
    static Consumable createSmokeEscapeVial();
};

#endif
