#include "combat/SystemeDegats.hpp"

#include "entite/Joueur.hpp"

#include <iostream>

int SystemeDegats::appliquerProtectionArmure(Entite& defenseur, int degatsBruts)
{
    Joueur* joueurDefenseur = dynamic_cast<Joueur*>(&defenseur);

    if (joueurDefenseur == nullptr || !joueurDefenseur->aUneArmureEquipee())
    {
        return degatsBruts;
    }

    Armure* armure = joueurDefenseur->getInventaire().getArmureModifiable(
        joueurDefenseur->getIndexArmureEquipee()
    );

    if (armure == nullptr || armure->estCassee())
    {
        return degatsBruts;
    }

    int absorption = armure->getReductionDegats();

    if (absorption <= 0)
    {
        return degatsBruts;
    }

    if (absorption > degatsBruts)
    {
        absorption = degatsBruts;
    }

    armure->perdreDurabilite(1);

    std::cout << "L'armure de " << defenseur.getNom() << " absorbe "
              << absorption << " dégâts." << std::endl;

    if (armure->estCassee())
    {
        std::cout << armure->getNom() << " se fissure sous l'impact et se brise." << std::endl;
        std::cout << "Elle ne protégera plus son porteur tant qu'elle ne sera pas réparée." << std::endl;
    }

    return degatsBruts - absorption;
}