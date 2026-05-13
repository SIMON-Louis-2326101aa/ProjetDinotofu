#include "combat/SystemeDegats.hpp"
#include "combat/SystemeClassesCombat.hpp"

#include "entite/Joueur.hpp"

#include <iostream>

int SystemeDegats::appliquerProtectionArmure(Entite& defenseur, int degatsBruts)
{
    int degatsApresArmure = degatsBruts;

    Joueur* joueurDefenseur = dynamic_cast<Joueur*>(&defenseur);

    if (joueurDefenseur != nullptr && joueurDefenseur->aUneArmureEquipee())
    {
        Armure* armure = joueurDefenseur->getInventaire().getArmureModifiable(
            joueurDefenseur->getIndexArmureEquipee()
        );

        if (armure != nullptr && !armure->estCassee())
        {
            int absorptionArmure = armure->getReductionDegats();

            if (absorptionArmure > 0)
            {
                if (absorptionArmure > degatsApresArmure)
                {
                    absorptionArmure = degatsApresArmure;
                }

                armure->perdreDurabilite(1);

                degatsApresArmure -= absorptionArmure;

                std::cout << "L'armure de " << defenseur.getNom() << " absorbe "
                          << absorptionArmure << " dégâts." << std::endl;

                if (armure->estCassee())
                {
                    std::cout << armure->getNom()
                              << " se fissure sous l'impact et se brise."
                              << std::endl;
                    std::cout << "Elle ne protégera plus son porteur tant qu'elle ne sera pas réparée."
                              << std::endl;
                }
            }
        }
    }

    int reductionClassePourcentage =
        SystemeClassesCombat::getReductionDegatsBasePourcentage(defenseur);

    if (reductionClassePourcentage > 0 && degatsApresArmure > 0)
    {
        int absorptionClasse = degatsApresArmure * reductionClassePourcentage / 100;

        if (absorptionClasse > 0)
        {
            degatsApresArmure -= absorptionClasse;

            std::cout << defenseur.getNom()
                      << " réduit naturellement "
                      << absorptionClasse
                      << " dégâts grâce à sa résistance de classe."
                      << std::endl;
        }
    }

    if (degatsApresArmure < 0)
    {
        degatsApresArmure = 0;
    }

    return degatsApresArmure;
}