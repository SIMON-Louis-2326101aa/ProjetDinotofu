// EN: DefensePostureSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: DefensePostureSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/system/DefensePostureSystem.hpp"

#include "combat/DamageReport.hpp"
#include "combat/system/DamageSystem.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    std::string normalize(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c)
        {
            return static_cast<char>(std::tolower(c));
        });

        std::string::size_type position = 0;
        while ((position = value.find("é", position)) != std::string::npos)
        {
            value.replace(position, 2, "e");
            position += 1;
        }

        position = 0;
        while ((position = value.find("ô", position)) != std::string::npos)
        {
            value.replace(position, 2, "o");
            position += 1;
        }

        return value;
    }
}

// EN: containsClassText declares or implements a focused behavior used by this module.
// FR: containsClassText déclare ou implémente un comportement précis utilisé par ce module.
bool DefensePostureSystem::containsClassText(const Entity& entity, const std::string& searched)
{
    return normalize(entity.getType()).find(normalize(searched)) != std::string::npos;
}

// EN: isMeleeClass declares or implements a focused behavior used by this module.
// FR: isMeleeClass déclare ou implémente un comportement précis utilisé par ce module.
bool DefensePostureSystem::isMeleeClass(const Entity& entity)
{
    return containsClassText(entity, "chevalier")
        || containsClassText(entity, "assassin")
        || containsClassText(entity, "colosse")
        || containsClassText(entity, "barbare")
        || containsClassText(entity, "guerrier")
        || containsClassText(entity, "moine")
        || containsClassText(entity, "duelliste")
        || containsClassText(entity, "berserker")
        || containsClassText(entity, "lancier")
        || containsClassText(entity, "briseur")
        || containsClassText(entity, "paladin")
        || containsClassText(entity, "gardien")
        || containsClassText(entity, "templier")
        || containsClassText(entity, "tank")
        || containsClassText(entity, "forgeron");
}

// EN: isDistanceClass declares or implements a focused behavior used by this module.
// FR: isDistanceClass déclare ou implémente un comportement précis utilisé par ce module.
bool DefensePostureSystem::isDistanceClass(const Entity& entity)
{
    return containsClassText(entity, "archer")
        || containsClassText(entity, "rodeur")
        || containsClassText(entity, "rôdeur")
        || containsClassText(entity, "arbaletrier")
        || containsClassText(entity, "arbalétrier")
        || containsClassText(entity, "chasseur")
        || containsClassText(entity, "lanceur de dagues")
        || containsClassText(entity, "tireur")
        || containsClassText(entity, "artificier");
}

// EN: isCounterMeleeClass declares or implements a focused behavior used by this module.
// FR: isCounterMeleeClass déclare ou implémente un comportement précis utilisé par ce module.
bool DefensePostureSystem::isCounterMeleeClass(const Entity& entity)
{
    return containsClassText(entity, "duelliste")
        || containsClassText(entity, "lancier")
        || containsClassText(entity, "moine")
        || containsClassText(entity, "assassin")
        || containsClassText(entity, "chevalier")
        || containsClassText(entity, "guerrier")
        || containsClassText(entity, "paladin")
        || containsClassText(entity, "templier");
}

// EN: isCounterDistanceClass declares or implements a focused behavior used by this module.
// FR: isCounterDistanceClass déclare ou implémente un comportement précis utilisé par ce module.
bool DefensePostureSystem::isCounterDistanceClass(const Entity& entity)
{
    return containsClassText(entity, "archer")
        || containsClassText(entity, "rodeur")
        || containsClassText(entity, "rôdeur")
        || containsClassText(entity, "arbaletrier")
        || containsClassText(entity, "arbalétrier")
        || containsClassText(entity, "tireur")
        || containsClassText(entity, "chasseur")
        || containsClassText(entity, "artificier");
}

// EN: enterDefensePosture declares or implements a focused behavior used by this module.
// FR: enterDefensePosture déclare ou implémente un comportement précis utilisé par ce module.
void DefensePostureSystem::enterDefensePosture(Entity& entity)
{
    int reduction = 22;
    int counterChance = 8;
    std::string label = "Posture prudente";

    if (isMeleeClass(entity))
    {
        reduction = 35;
        counterChance = isCounterMeleeClass(entity) ? 22 : 12;
        label = "Posture de garde rapprochée";
    }
    else if (isDistanceClass(entity))
    {
        reduction = 16;
        counterChance = isCounterDistanceClass(entity) ? 28 : 16;
        label = "Posture de recul";
    }

    entity.startDefensePosture(reduction, counterChance, label);

    std::vector<std::string> lines;
    lines.push_back(entity.getName() + " adopte une " + label + ".");
    lines.push_back("Réduction du prochain coup reçu : " + std::to_string(reduction) + "%");

    if (counterChance > 0)
    {
        lines.push_back("Si l'ennemi rate pendant cette posture, une contre-attaque peut partir.");
        lines.push_back("Chance de contre : " + std::to_string(counterChance) + "%");
    }

    MessageScreen::show("POSTURE DÉFENSIVE", "combat.defense_posture.enter", lines, false);
}

// EN: reduceIncomingDamage declares or implements a focused behavior used by this module.
// FR: reduceIncomingDamage déclare ou implémente un comportement précis utilisé par ce module.
int DefensePostureSystem::reduceIncomingDamage(Entity& defender, int receivedDamage)
{
    if (!defender.isInDefensePosture())
    {
        return receivedDamage;
    }

    int reduction = defender.getDefenseReductionPercent();
    int reducedDamage = receivedDamage * (100 - reduction) / 100;

    if (receivedDamage > 0 && reducedDamage < 1)
    {
        reducedDamage = 1;
    }

    int blocked = receivedDamage - reducedDamage;

    MessageScreen::show(
        "POSTURE TENUE",
        "combat.defense_posture.reduce",
        {
            defender.getName() + " tient sa posture défensive.",
            "Dégâts entrants : " + std::to_string(receivedDamage),
            "Dégâts bloqués : " + std::to_string(blocked),
            "Dégâts restants : " + std::to_string(reducedDamage)
        },
        false
    );

    defender.clearDefensePosture();
    return reducedDamage;
}

// EN: tryCounterAfterMiss declares or implements a focused behavior used by this module.
// FR: tryCounterAfterMiss déclare ou implémente un comportement précis utilisé par ce module.
void DefensePostureSystem::tryCounterAfterMiss(Entity& defender, Entity& attacker, Random& random)
{
    if (!defender.isInDefensePosture())
    {
        return;
    }

    int counterChance = defender.getDefenseCounterChance();
    std::string label = defender.getDefensePostureLabel();
    defender.clearDefensePosture();

    if (counterChance <= 0 || random.between(1, 100) > counterChance)
    {
        MessageScreen::show(
            "CONTRE IMPOSSIBLE",
            "combat.defense_posture.counter.missed_window",
            {
                defender.getName() + " garde sa posture, mais ne trouve pas l'ouverture pour contrer."
            },
            false
        );
        return;
    }

    bool dodged = false;
    bool critical = false;
    int rawDamage = defender.attack(random, dodged, critical, 0);

    if (dodged || rawDamage <= 0)
    {
        MessageScreen::show(
            "CONTRE RATÉ",
            "combat.defense_posture.counter.failed",
            {
                defender.getName() + " tente de contre-attaquer depuis sa " + label + ", mais le geste part trop tard."
            },
            false
        );
        return;
    }

    DamageReport report = DamageSystem::calculateReceivedDamage(attacker, rawDamage);
    DamageSystem::displayDamageReport(attacker, report);
    attacker.takeDamage(report.receivedDamage);

    MessageScreen::show(
        "CONTRE-ATTAQUE",
        "combat.defense_posture.counter.success",
        {
            defender.getName() + " profite du raté pour contre-attaquer depuis sa " + label + ".",
            attacker.getName() + " reçoit " + std::to_string(report.receivedDamage) + " dégâts de contre.",
            attacker.getName() + " possède maintenant " + std::to_string(attacker.getHp()) + "/" + std::to_string(attacker.getMaxHp()) + " PV."
        },
        false
    );
}
