// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/consumable/ConsumableCatalog.hpp"

Consumable ConsumableCatalog::creerPotionSoinBasique()
{
    return Consumable(
        "Potion de soin",
        "Referme les blessures et redonne un souffle de vie.",
        25,
        ConsumableType::Healing,
        55
    );
}

Consumable ConsumableCatalog::creerPotionDegatsBasique()
{
    return Consumable(
        "Potion de rage",
        "Fait monter la puissance d'un coup, mais force à attaquer immédiatement.",
        30,
        ConsumableType::Damage,
        25
    );
}

Consumable ConsumableCatalog::creerPotionSoinRenforcee()
{
    return Consumable(
        "Potion de soin renforcée",
        "Une potion plus rare, utilisée lorsque l'arène décide que le combat doit durer.",
        60,
        ConsumableType::Healing,
        75
    );
}

Consumable ConsumableCatalog::creerPotionDegatsRenforcee()
{
    return Consumable(
        "Potion de rage supérieure",
        "Une rage plus pure, plus brutale, presque trop violente pour un simple humain.",
        70,
        ConsumableType::Damage,
        45
    );
}