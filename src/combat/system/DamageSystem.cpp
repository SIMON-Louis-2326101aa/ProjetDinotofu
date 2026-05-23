// EN: DamageSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: DamageSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/system/DamageSystem.hpp"
#include "combat/system/CombatClassSystem.hpp"

#include "entity/Player.hpp"
#include "entity/Monster.hpp"

#include <iostream>

// EN: calculateReceivedDamage declares or implements a focused behavior used by this module.
// FR: calculateReceivedDamage déclare ou implémente un comportement précis utilisé par ce module.
DamageReport DamageSystem::calculateReceivedDamage(Entity& defender, int rawDamage)
{
    DamageReport rapport;
    rapport.rawDamage = rawDamage;

    int remainingDamage = rawDamage;

    Player* defendingPlayer = dynamic_cast<Player*>(&defender);

    if (defendingPlayer != nullptr && defendingPlayer->hasEquippedArmor() && !defendingPlayer->hasBossEquipmentSeal())
    {
        Armor* armor = defendingPlayer->getInventory().getMutableArmor(
            defendingPlayer->getEquippedArmorIndex()
        );

        if (armor != nullptr && !armor->isBroken())
        {
            int armorAbsorption = armor->getDamageReduction();

            if (armorAbsorption > remainingDamage)
            {
                armorAbsorption = remainingDamage;
            }

            if (armorAbsorption > 0)
            {
                rapport.armorUsed = true;
                rapport.armorAbsorbedDamage = armorAbsorption;

                if (!defendingPlayer->hasIndestructibleEquipment())
                {
                    armor->loseDurability(1);
                }

                if (armor->isBroken())
                {
                    rapport.armorBrokenDuringImpact = true;
                }

                remainingDamage -= armorAbsorption;
            }
        }
    }

    if (defendingPlayer != nullptr && defendingPlayer->hasEquippedArmor() && defendingPlayer->hasBossEquipmentSeal())
    {
        std::cout << "Le sceau de boss empêche l'armure de " << defendingPlayer->getName()
                  << " de répondre correctement." << std::endl;
        std::cout << std::endl;
    }

    int classReductionPercentage =
        CombatClassSystem::getBaseDamageReductionPercentage(defender);

    Monster* defendingMonster = dynamic_cast<Monster*>(&defender);
    if (defendingMonster != nullptr)
    {
        const std::string raceText = defendingMonster->getRaceText();
        const std::string typeText = defendingMonster->getType();

        if (raceText.find("Slime") != std::string::npos)
        {
            classReductionPercentage += 6;
        }
        else if (raceText.find("Plante") != std::string::npos || typeText.find("résistante") != std::string::npos)
        {
            classReductionPercentage += 4;
        }
        else if (raceText.find("Mort-vivant") != std::string::npos)
        {
            classReductionPercentage += 5;
        }
        else if (raceText.find("Dragon") != std::string::npos || raceText.find("Draconide") != std::string::npos)
        {
            classReductionPercentage += 8;
        }

        if (defendingMonster->isElite())
        {
            classReductionPercentage += 3;
        }
    }

    if (classReductionPercentage > 0 && remainingDamage > 0)
    {
        int classReduction = remainingDamage * classReductionPercentage / 100;

        if (classReduction > remainingDamage)
        {
            classReduction = remainingDamage;
        }

        if (classReduction > 0)
        {
            rapport.classReducedDamage = classReduction;
            remainingDamage -= classReduction;
        }
    }

    if (remainingDamage < 0)
    {
        remainingDamage = 0;
    }

    rapport.receivedDamage = remainingDamage;

    return rapport;
}

// EN: displayDamageReport declares or implements a focused behavior used by this module.
// FR: displayDamageReport déclare ou implémente un comportement précis utilisé par ce module.
void DamageSystem::displayDamageReport(const Entity& defender, const DamageReport& rapport)
{
    if (rapport.armorAbsorbedDamage > 0)
    {
        std::cout << "L'armure de " << defender.getName()
                  << " absorbe " << rapport.armorAbsorbedDamage
                  << " dégâts."
                  << std::endl;
    }
    else if (rapport.armorUsed)
    {
        std::cout << "L'armure de " << defender.getName()
                  << " encaisse le choc, mais n'absorbe aucun dégât."
                  << std::endl;
    }

    if (rapport.armorBrokenDuringImpact)
    {
        std::cout << "L'armure se fissure sous l'impact et se brise." << std::endl;
        std::cout << "Elle ne protégera plus son porteur tant qu'elle ne sera pas réparée."
                  << std::endl;
    }

    if (rapport.classReducedDamage > 0)
    {
        std::cout << defender.getName()
                  << " réduit naturellement "
                  << rapport.classReducedDamage
                  << " dégâts grâce à sa résistance de classe."
                  << std::endl;
    }
}

// EN: applyArmorProtection declares or implements a focused behavior used by this module.
// FR: applyArmorProtection déclare ou implémente un comportement précis utilisé par ce module.
int DamageSystem::applyArmorProtection(Entity& defender, int rawDamage)
{
    DamageReport rapport = calculateReceivedDamage(defender, rawDamage);
    displayDamageReport(defender, rapport);

    return rapport.receivedDamage;
}
