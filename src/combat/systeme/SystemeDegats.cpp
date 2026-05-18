#include "combat/systeme/SystemeDegats.hpp"
#include "combat/systeme/SystemeClassesCombat.hpp"

#include "entite/Joueur.hpp"

#include <iostream>

RapportDegats SystemeDegats::calculerDegatsRecus(Entite& defenseur, int degatsBruts)
{
    RapportDegats rapport;
    rapport.degatsBruts = degatsBruts;

    int degatsRestants = degatsBruts;

    Joueur* joueurDefenseur = dynamic_cast<Joueur*>(&defenseur);

    if (joueurDefenseur != nullptr && joueurDefenseur->aUneArmureEquipee())
    {
        Armure* armure = joueurDefenseur->getInventaire().getArmureModifiable(
            joueurDefenseur->getIndexArmureEquipee()
        );

        if (armure != nullptr && !armure->estCassee())
        {
            int absorptionArmure = armure->getReductionDegats();

            if (absorptionArmure > degatsRestants)
            {
                absorptionArmure = degatsRestants;
            }

            if (absorptionArmure > 0)
            {
                rapport.armureUtilisee = true;
                rapport.degatsAbsorbesArmure = absorptionArmure;

                armure->perdreDurabilite(1);

                if (armure->estCassee())
                {
                    rapport.armureCasseePendantImpact = true;
                }

                degatsRestants -= absorptionArmure;
            }
        }
    }

    int reductionClassePourcentage =
        SystemeClassesCombat::getReductionDegatsBasePourcentage(defenseur);

    if (reductionClassePourcentage > 0 && degatsRestants > 0)
    {
        int reductionClasse = degatsRestants * reductionClassePourcentage / 100;

        if (reductionClasse > degatsRestants)
        {
            reductionClasse = degatsRestants;
        }

        if (reductionClasse > 0)
        {
            rapport.degatsReduitsClasse = reductionClasse;
            degatsRestants -= reductionClasse;
        }
    }

    if (degatsRestants < 0)
    {
        degatsRestants = 0;
    }

    rapport.degatsRecus = degatsRestants;

    return rapport;
}

void SystemeDegats::afficherRapportDegats(const Entite& defenseur, const RapportDegats& rapport)
{
    if (rapport.degatsAbsorbesArmure > 0)
    {
        std::cout << "L'armure de " << defenseur.getNom()
                  << " absorbe " << rapport.degatsAbsorbesArmure
                  << " dégâts."
                  << std::endl;
    }
    else if (rapport.armureUtilisee)
    {
        std::cout << "L'armure de " << defenseur.getNom()
                  << " encaisse le choc, mais n'absorbe aucun dégât."
                  << std::endl;
    }

    if (rapport.armureCasseePendantImpact)
    {
        std::cout << "L'armure se fissure sous l'impact et se brise." << std::endl;
        std::cout << "Elle ne protégera plus son porteur tant qu'elle ne sera pas réparée."
                  << std::endl;
    }

    if (rapport.degatsReduitsClasse > 0)
    {
        std::cout << defenseur.getNom()
                  << " réduit naturellement "
                  << rapport.degatsReduitsClasse
                  << " dégâts grâce à sa résistance de classe."
                  << std::endl;
    }
}

int SystemeDegats::appliquerProtectionArmure(Entite& defenseur, int degatsBruts)
{
    RapportDegats rapport = calculerDegatsRecus(defenseur, degatsBruts);
    afficherRapportDegats(defenseur, rapport);

    return rapport.degatsRecus;
}