// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/system/DamageSystem.hpp"
#include "combat/system/CombatClassSystem.hpp"

#include "entity/Player.hpp"

#include <iostream>

DamageReport DamageSystem::calculateReceivedDamage(Entity& defender, int rawDamage)
{
    DamageReport rapport;
    rapport.rawDamage = rawDamage;

    int remainingDamage = rawDamage;

    Player* defendingPlayer = dynamic_cast<Player*>(&defender);

    if (defendingPlayer != nullptr && defendingPlayer->hasEquippedArmor())
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

    int classReductionPercentage =
        CombatClassSystem::getBaseDamageReductionPercentage(defender);

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

int DamageSystem::applyArmorProtection(Entity& defender, int rawDamage)
{
    DamageReport rapport = calculateReceivedDamage(defender, rawDamage);
    displayDamageReport(defender, rapport);

    return rapport.receivedDamage;
}