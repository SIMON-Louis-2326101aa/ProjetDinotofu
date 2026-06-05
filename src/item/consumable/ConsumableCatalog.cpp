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

Consumable ConsumableCatalog::createVitalityHealingPotion()
{
    return Consumable(
        "Potion de vitalité proportionnelle",
        "Soin stable qui récupère un pourcentage des PV max. Plus utile sur les personnages robustes que sur les blessés fragiles.",
        155,
        ConsumableType::Healing,
        35,
        true
    );
}

Consumable ConsumableCatalog::createRoyalVitalityHealingPotion()
{
    return Consumable(
        "Potion de vitalité royale",
        "Soin rare à pourcentage élevé. Une fiole chère, pensée pour les tanks et les combats où les PV max comptent vraiment.",
        340,
        ConsumableType::Healing,
        60,
        true
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
        "Potion de préparation défensive. Son mélange durcit brièvement les réflexes du corps.",
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
        "Potion offensive à lancer sur une cible. Son mélange épuise la force du prochain échange.",
        85,
        ConsumableType::Debuff,
        22
    );
}

Consumable ConsumableCatalog::createFragilityDebuffPotion()
{
    return Consumable(
        "Fiole de fragilisation",
        "Potion offensive à lancer sur une cible. Son mélange ouvre une faille qui rend les prochains impacts plus cruels.",
        105,
        ConsumableType::Debuff,
        26
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

Consumable ConsumableCatalog::createElementalWardPotion()
{
    return Consumable(
        "Potion de voile élémentaire",
        "Une potion défensive qui enveloppe le corps d'un voile court contre les altérations élémentaires.",
        120,
        ConsumableType::Buff,
        30
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

Consumable ConsumableCatalog::createArcaneSparkScroll()
{
    return Consumable(
        "Parchemin d'étincelle arcanique",
        "Un sort scellé dans l'encre. Même un non-mage peut le déclencher, mais une seule fois.",
        95,
        ConsumableType::Special,
        24
    );
}

Consumable ConsumableCatalog::createElementalWardScroll()
{
    return Consumable(
        "Parchemin de voile élémentaire",
        "Un voile défensif inscrit sur papier runique. Usage unique, précieux quand aucun vrai mage n'est disponible.",
        135,
        ConsumableType::Special,
        28
    );
}

Consumable ConsumableCatalog::createResistanceRiftScroll()
{
    return Consumable(
        "Parchemin de faille fragile",
        "Une formule instable qui ouvre brièvement la défense d'une cible. Usage unique et risqué.",
        185,
        ConsumableType::Special,
        32
    );
}

Consumable ConsumableCatalog::createWanderingEmberScroll()
{
    return Consumable(
        "Parchemin de braise errante",
        "Une magie jetable sans grimoire stable connu. La braise choisit mal sa faim si la main tremble.",
        125,
        ConsumableType::Special,
        26
    );
}

Consumable ConsumableCatalog::createMinorPurificationScroll()
{
    return Consumable(
        "Parchemin de purification mineure",
        "Une magie jetable qui chasse quelques altérations. Aucun grimoire stable courant ne reproduit exactement ce tracé.",
        115,
        ConsumableType::Special,
        22
    );
}

Consumable ConsumableCatalog::createCrawlingVenomScroll()
{
    return Consumable(
        "Parchemin de venin rampant",
        "Une formule jetable de venin magique. Elle existe surtout sur papier : le tracé se dissout dès que le poison mord.",
        145,
        ConsumableType::Special,
        25
    );
}

Consumable ConsumableCatalog::createStabilizingTea()
{
    return Consumable("Thé stabilisant", "Infusion calme utilisée avant les sorties longues. Petit soin, petit ancrage, zéro héroïsme inutile.", 48, ConsumableType::Healing, 34);
}

Consumable ConsumableCatalog::createMinerBracingTonic()
{
    return Consumable("Tonique de mineur", "Tonique épais qui aide à encaisser un coup lourd ou une chute. Pas bon, mais efficace.", 82, ConsumableType::Buff, 27);
}

Consumable ConsumableCatalog::createCartographerFocusInk()
{
    return Consumable("Encre de concentration", "Petite fiole à respirer ou verser sur un outil. Aide les gestes précis, tirs, croquis et pièges.", 92, ConsumableType::Buff, 29);
}

Consumable ConsumableCatalog::createMoonSaltPurifier()
{
    return Consumable("Purificateur au sel lunaire", "Mélange pâle qui nettoie certaines altérations faibles et rend un souffle de PV.", 118, ConsumableType::Healing, 46);
}

Consumable ConsumableCatalog::createGlassStepScroll()
{
    return Consumable("Parchemin de pas de verre", "Sort jetable d'évitement bref : pratique pour survivre à un mauvais engagement, mais le tracé se brise aussitôt.", 170, ConsumableType::Special, 31);
}

Consumable ConsumableCatalog::createVineSnareScroll()
{
    return Consumable("Parchemin de ronce-lien", "Sort jetable végétal. Il gêne une cible, surtout si elle bouge trop ou compte fuir proprement.", 160, ConsumableType::Special, 30);
}


Consumable ConsumableCatalog::createArchivistFocusInk()
{
    return Consumable("Encre de concentration d'archiviste", "Encre à respirer très brièvement avant une lecture ou un rapport. Aide la précision, mais donne envie de corriger tout le monde.", 105, ConsumableType::Buff, 31);
}

Consumable ConsumableCatalog::createCliffBasilTea()
{
    return Consumable("Tisane de basilic des falaises", "Infusion salée qui calme le vertige, rend quelques PV et stabilise une sortie dangereuse.", 88, ConsumableType::Healing, 42);
}

Consumable ConsumableCatalog::createCarnivalDiversionTicket()
{
    return Consumable("Ticket de diversion de foire", "Objet jetable étrange : claque le ticket, détourne l'attention et offre une petite fenêtre défensive ou de fuite.", 150, ConsumableType::Special, 28);
}

Consumable ConsumableCatalog::createFireflyGuardVial()
{
    return Consumable("Fiole de garde-lucioles", "Fiole lumineuse qui brouille les attaques faibles et donne un court appui défensif aux explorateurs prudents.", 132, ConsumableType::Buff, 33);
}
