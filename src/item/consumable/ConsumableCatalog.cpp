// EN: ConsumableCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: ConsumableCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/consumable/ConsumableCatalog.hpp"

// EN: createBasicHealingPotion declares or implements a focused behavior used by this module.
// FR: createBasicHealingPotion déclare ou implémente un comportement précis utilisé par ce module.
Consumable ConsumableCatalog::createBasicHealingPotion()
{
    return Consumable(
        "Potion de soin",
        "Referme les blessures et redonne un souffle de vie.",
        25,
        ConsumableType::Healing,
        55
    );
}

// EN: createBasicDamagePotion declares or implements a focused behavior used by this module.
// FR: createBasicDamagePotion déclare ou implémente un comportement précis utilisé par ce module.
Consumable ConsumableCatalog::createBasicDamagePotion()
{
    return Consumable(
        "Potion de rage",
        "Fait monter la puissance d'un coup, mais force à attaquer immédiatement.",
        30,
        ConsumableType::Damage,
        25
    );
}

// EN: createReinforcedHealingPotion declares or implements a focused behavior used by this module.
// FR: createReinforcedHealingPotion déclare ou implémente un comportement précis utilisé par ce module.
Consumable ConsumableCatalog::createReinforcedHealingPotion()
{
    return Consumable(
        "Potion de soin renforcée",
        "Une potion plus rare, utilisée lorsque l'arène décide que le combat doit durer.",
        60,
        ConsumableType::Healing,
        75
    );
}

// EN: createReinforcedDamagePotion declares or implements a focused behavior used by this module.
// FR: createReinforcedDamagePotion déclare ou implémente un comportement précis utilisé par ce module.
Consumable ConsumableCatalog::createReinforcedDamagePotion()
{
    return Consumable(
        "Potion de rage supérieure",
        "Une rage plus pure, plus brutale, presque trop violente pour un simple humain.",
        70,
        ConsumableType::Damage,
        45
    );
}
