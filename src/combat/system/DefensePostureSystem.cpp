// EN: DefensePostureSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: DefensePostureSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/system/DefensePostureSystem.hpp"

#include "combat/DamageReport.hpp"
#include "combat/system/DamageSystem.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>

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

    std::cout << entity.getName() << " adopte une " << label << "." << std::endl;
    std::cout << "Réduction du prochain coup reçu : " << reduction << "%" << std::endl;

    if (counterChance > 0)
    {
        std::cout << "Si l'ennemi rate pendant cette posture, une contre-attaque peut partir." << std::endl;
        std::cout << "Chance de contre : " << counterChance << "%" << std::endl;
    }

    std::cout << std::endl;
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

    std::cout << defender.getName() << " tient sa posture défensive." << std::endl;
    std::cout << "Dégâts bloqués par la posture : " << blocked << std::endl;
    std::cout << std::endl;

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
        std::cout << defender.getName() << " garde sa posture, mais ne trouve pas l'ouverture pour contrer." << std::endl;
        std::cout << std::endl;
        return;
    }

    bool dodged = false;
    bool critical = false;
    int rawDamage = defender.attack(random, dodged, critical, 0);

    if (dodged || rawDamage <= 0)
    {
        std::cout << defender.getName() << " tente de contre-attaquer depuis sa " << label
                  << ", mais le geste part trop tard." << std::endl;
        std::cout << std::endl;
        return;
    }

    std::cout << defender.getName() << " profite du raté pour contre-attaquer depuis sa "
              << label << "." << std::endl;

    DamageReport report = DamageSystem::calculateReceivedDamage(attacker, rawDamage);
    DamageSystem::displayDamageReport(attacker, report);
    attacker.takeDamage(report.receivedDamage);

    std::cout << attacker.getName() << " reçoit " << report.receivedDamage
              << " dégâts de contre." << std::endl;
    std::cout << attacker.getName() << " possède maintenant "
              << attacker.getHp() << "/" << attacker.getMaxHp() << " PV." << std::endl;
    std::cout << std::endl;
}
