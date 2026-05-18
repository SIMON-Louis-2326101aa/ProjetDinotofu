// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/system/DamageSystem.hpp"
#include "combat/system/CombatClassSystem.hpp"

#include "entity/Player.hpp"

#include <iostream>

DamageReport DamageSystem::calculerDegatsRecus(Entity& defender, int rawDamage)
{
    DamageReport rapport;
    rapport.rawDamage = rawDamage;

    int degatsRestants = rawDamage;

    Player* joueurDefenseur = dynamic_cast<Player*>(&defender);

    if (joueurDefenseur != nullptr && joueurDefenseur->hasEquippedArmor())
    {
        Armor* armor = joueurDefenseur->getInventory().getMutableArmor(
            joueurDefenseur->getEquippedArmorIndex()
        );

        if (armor != nullptr && !armor->isBroken())
        {
            int absorptionArmure = armor->getDamageReduction();

            if (absorptionArmure > degatsRestants)
            {
                absorptionArmure = degatsRestants;
            }

            if (absorptionArmure > 0)
            {
                rapport.armorUsed = true;
                rapport.armorAbsorbedDamage = absorptionArmure;

                armor->loseDurability(1);

                if (armor->isBroken())
                {
                    rapport.armorBrokenDuringImpact = true;
                }

                degatsRestants -= absorptionArmure;
            }
        }
    }

    int reductionClassePourcentage =
        CombatClassSystem::getReductionDegatsBasePourcentage(defender);

    if (reductionClassePourcentage > 0 && degatsRestants > 0)
    {
        int reductionClasse = degatsRestants * reductionClassePourcentage / 100;

        if (reductionClasse > degatsRestants)
        {
            reductionClasse = degatsRestants;
        }

        if (reductionClasse > 0)
        {
            rapport.classReducedDamage = reductionClasse;
            degatsRestants -= reductionClasse;
        }
    }

    if (degatsRestants < 0)
    {
        degatsRestants = 0;
    }

    rapport.receivedDamage = degatsRestants;

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

int DamageSystem::appliquerProtectionArmure(Entity& defender, int rawDamage)
{
    DamageReport rapport = calculerDegatsRecus(defender, rawDamage);
    displayDamageReport(defender, rapport);

    return rapport.receivedDamage;
}