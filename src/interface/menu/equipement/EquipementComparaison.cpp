#include "interface/menu/equipement/EquipementComparaison.hpp"

#include "interface/menu/equipement/EquipementAffichage.hpp"

#include <iostream>

void EquipementComparaison::afficherComparaisonArme(
    const Joueur& joueur,
    const Arme& nouvelleArme
)
{
    std::cout << "=========== COMPARAISON D'ARME ===========" << std::endl;

    if (joueur.aUneArmeEquipee())
    {
        Arme armeActuelle = joueur.getArmeEquipee();

        std::cout << "Arme actuelle : " << armeActuelle.getNom() << std::endl;
        std::cout << "Dégâts bonus : +"
                  << armeActuelle.getBonusDegatsMin()
                  << " à +"
                  << armeActuelle.getBonusDegatsMax()
                  << std::endl;
        std::cout << "Critique bonus : +" << armeActuelle.getBonusCritique() << std::endl;
        std::cout << "Durabilité : "
                  << EquipementAffichage::durabiliteArmeTexte(armeActuelle)
                  << std::endl;

        if (armeActuelle.estCassee())
        {
            std::cout << "État : Cassée" << std::endl;
        }
        else
        {
            std::cout << "État : Utilisable" << std::endl;
        }

        std::cout << std::endl;
    }
    else
    {
        std::cout << "Arme actuelle : Aucune" << std::endl;
        std::cout << std::endl;
    }

    std::cout << "Nouvelle arme : " << nouvelleArme.getNom() << std::endl;
    std::cout << "Dégâts bonus : +"
              << nouvelleArme.getBonusDegatsMin()
              << " à +"
              << nouvelleArme.getBonusDegatsMax()
              << std::endl;
    std::cout << "Critique bonus : +" << nouvelleArme.getBonusCritique() << std::endl;
    std::cout << "Durabilité : "
              << EquipementAffichage::durabiliteArmeTexte(nouvelleArme)
              << std::endl;

    if (nouvelleArme.estCassee())
    {
        std::cout << "État : Cassée" << std::endl;
    }
    else
    {
        std::cout << "État : Utilisable" << std::endl;
    }

    if (joueur.aUneArmeEquipee())
    {
        Arme armeActuelle = joueur.getArmeEquipee();

        std::cout << std::endl;
        std::cout << "Différence dégâts min : "
                  << nouvelleArme.getBonusDegatsMin() - armeActuelle.getBonusDegatsMin()
                  << std::endl;
        std::cout << "Différence dégâts max : "
                  << nouvelleArme.getBonusDegatsMax() - armeActuelle.getBonusDegatsMax()
                  << std::endl;
        std::cout << "Différence critique : "
                  << nouvelleArme.getBonusCritique() - armeActuelle.getBonusCritique()
                  << std::endl;
    }

    std::cout << "===========================================" << std::endl;
    std::cout << std::endl;
}

void EquipementComparaison::afficherComparaisonArmure(
    const Joueur& joueur,
    const Armure& nouvelleArmure
)
{
    std::cout << "========== COMPARAISON D'ARMURE ==========" << std::endl;

    if (joueur.aUneArmureEquipee())
    {
        Armure armureActuelle = joueur.getArmureEquipee();

        std::cout << "Armure actuelle : " << armureActuelle.getNom() << std::endl;
        std::cout << "Bonus PV max : +" << armureActuelle.getBonusPvMax() << std::endl;
        std::cout << "Réduction dégâts : " << armureActuelle.getReductionDegats() << std::endl;
        std::cout << "Durabilité : "
                  << EquipementAffichage::durabiliteArmureTexte(armureActuelle)
                  << std::endl;

        if (armureActuelle.estCassee())
        {
            std::cout << "État : Cassée" << std::endl;
        }
        else
        {
            std::cout << "État : Utilisable" << std::endl;
        }

        std::cout << std::endl;
    }
    else
    {
        std::cout << "Armure actuelle : Aucune" << std::endl;
        std::cout << std::endl;
    }

    std::cout << "Nouvelle armure : " << nouvelleArmure.getNom() << std::endl;
    std::cout << "Bonus PV max : +" << nouvelleArmure.getBonusPvMax() << std::endl;
    std::cout << "Réduction dégâts : " << nouvelleArmure.getReductionDegats() << std::endl;
    std::cout << "Durabilité : "
              << EquipementAffichage::durabiliteArmureTexte(nouvelleArmure)
              << std::endl;

    if (nouvelleArmure.estCassee())
    {
        std::cout << "État : Cassée" << std::endl;
    }
    else
    {
        std::cout << "État : Utilisable" << std::endl;
    }

    if (joueur.aUneArmureEquipee())
    {
        Armure armureActuelle = joueur.getArmureEquipee();

        std::cout << std::endl;
        std::cout << "Différence PV max : "
                  << nouvelleArmure.getBonusPvMax() - armureActuelle.getBonusPvMax()
                  << std::endl;
        std::cout << "Différence réduction : "
                  << nouvelleArmure.getReductionDegats() - armureActuelle.getReductionDegats()
                  << std::endl;
    }

    std::cout << "===========================================" << std::endl;
    std::cout << std::endl;
}