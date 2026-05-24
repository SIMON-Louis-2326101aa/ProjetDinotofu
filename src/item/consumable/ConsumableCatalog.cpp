// EN: ConsumableCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: ConsumableCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/consumable/ConsumableCatalog.hpp"

// EN: createBasicHealingPotion declares or implements a focused behavior used by this module.
// FR: createBasicHealingPotion déclare ou implémente un comportement précis utilisé par ce module.
Consumable ConsumableCatalog::createMinorHealingPotion()
{
    return Consumable(
        "Petite potion de soin",
        "Un soin faible, souvent utilisé quand on n\'a pas mieux.",
        12,
        ConsumableType::Healing,
        28
    );
}

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
Consumable ConsumableCatalog::createMinorDamagePotion()
{
    return Consumable(
        "Petite potion de rage",
        "Une poussée offensive faible, mais parfois suffisante pour finir un ennemi.",
        15,
        ConsumableType::Damage,
        14
    );
}

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

Consumable ConsumableCatalog::createGreaterHealingPotion()
{
    return Consumable(
        "Potion de soin supérieure",
        "Une potion plus propre, plus dense, utile quand les ennemis commencent à frapper sérieusement.",
        105,
        ConsumableType::Healing,
        110
    );
}

Consumable ConsumableCatalog::createMajorHealingPotion()
{
    return Consumable(
        "Potion de soin majeure",
        "Une potion rare capable de sauver un personnage qui aurait dû tomber.",
        210,
        ConsumableType::Healing,
        175
    );
}

Consumable ConsumableCatalog::createGreaterDamagePotion()
{
    return Consumable(
        "Potion de rage majeure",
        "Une rage concentrée, chère, brutale, à ne pas boire pour impressionner quelqu'un.",
        145,
        ConsumableType::Damage,
        72
    );
}

Consumable ConsumableCatalog::createExperimentalDamagePotion()
{
    return Consumable(
        "Potion de rage expérimentale",
        "Un produit instable de marché noir. Très puissant, mais clairement pas homologué par une guilde saine.",
        320,
        ConsumableType::Damage,
        105
    );
}

Consumable ConsumableCatalog::createDefensivePotion()
{
    return Consumable(
        "Potion défensive",
        "Potion de préparation défensive. Le vrai effet défensif sera branché avec les buffs complets.",
        45,
        ConsumableType::Buff,
        18
    );
}

Consumable ConsumableCatalog::createGreaterDefensivePotion()
{
    return Consumable(
        "Potion défensive supérieure",
        "Version plus dense d'une potion défensive, pensée pour les tanks et gardiens.",
        115,
        ConsumableType::Buff,
        34
    );
}

Consumable ConsumableCatalog::createPrecisionPotion()
{
    return Consumable(
        "Potion de précision",
        "Potion de buff pour tirs, dagues et gestes minutieux. Les bonus actifs viendront avec les techniques.",
        70,
        ConsumableType::Buff,
        24
    );
}

Consumable ConsumableCatalog::createWeakeningDebuffPotion()
{
    return Consumable(
        "Fiole d'affaiblissement",
        "Potion offensive/debuff à lancer sur une cible. Prévue pour le futur sous-menu de potions offensives.",
        85,
        ConsumableType::Debuff,
        22
    );
}


Consumable ConsumableCatalog::createAntidotePotion()
{
    return Consumable(
        "Antidote simple",
        "Neutralise les poisons faibles en combat et rend quelques PV.",
        40,
        ConsumableType::Healing,
        20
    );
}

Consumable ConsumableCatalog::createBurnSalvePotion()
{
    return Consumable(
        "Baume anti-brûlure",
        "Apaise une brûlure faible en combat et rend quelques PV.",
        42,
        ConsumableType::Healing,
        18
    );
}

Consumable ConsumableCatalog::createFrostResistancePotion()
{
    return Consumable(
        "Potion tiède anti-givre",
        "Réchauffe le corps, retire un ralentissement de givre et donne une stabilisation défensive.",
        75,
        ConsumableType::Buff,
        22
    );
}

Consumable ConsumableCatalog::createShockResistancePotion()
{
    return Consumable(
        "Potion isolante",
        "Coupe un choc électrique actif et donne une stabilisation défensive, surtout utile avec du métal équipé.",
        90,
        ConsumableType::Buff,
        24
    );
}

Consumable ConsumableCatalog::createSmokeEscapeVial()
{
    return Consumable(
        "Fiole de fumée de secours",
        "Outil instable : ne garantit pas la fuite, mais casse la pression immédiate et donne une posture défensive.",
        180,
        ConsumableType::Special,
        1
    );
}
