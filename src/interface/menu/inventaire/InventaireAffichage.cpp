#include "interface/menu/inventaire/InventaireAffichage.hpp"

#include "interface/menu/inventaire/InventaireUtils.hpp"

#include "objet/Inventaire.hpp"
#include "objet/consommable/Consommable.hpp"

#include <iostream>
#include <vector>

void InventaireAffichage::afficherMenuPrincipal()
{
    std::cout << "================ INVENTAIRE ================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Voir tout" << std::endl;
    std::cout << "2 : Voir les armes" << std::endl;
    std::cout << "3 : Voir les armures" << std::endl;
    std::cout << "4 : Voir les consommables" << std::endl;
    std::cout << "5 : Voir les matériaux" << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void InventaireAffichage::afficherToutSimple(const Joueur& joueur)
{
    const Inventaire& inventaire = joueur.getInventaire();

    std::cout << "================ INVENTAIRE ================" << std::endl;
    std::cout << "Or : " << inventaire.getOr() << " pièces" << std::endl;
    std::cout << std::endl;

    std::cout << "Armes : " << inventaire.getNombreArmes() << std::endl;

    for (int i = 0; i < inventaire.getNombreArmes(); ++i)
    {
        Arme arme = inventaire.getArme(i);

        std::cout << "[" << i << "] " << arme.getNom()
                  << " | Durabilité : " << InventaireUtils::durabiliteArmeTexte(arme);

        if (arme.estCassee())
        {
            std::cout << " | Cassée";
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Armures : " << inventaire.getNombreArmures() << std::endl;

    for (int i = 0; i < inventaire.getNombreArmures(); ++i)
    {
        Armure armure = inventaire.getArmure(i);

        std::cout << "[" << i << "] " << armure.getNom()
                  << " | Durabilité : " << InventaireUtils::durabiliteArmureTexte(armure);

        if (armure.estCassee())
        {
            std::cout << " | Cassée";
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Consommables : " << inventaire.getNombreConsommables() << std::endl;

    std::vector<GroupeConsommable> groupes = InventaireUtils::grouperConsommables(joueur);

    for (int i = 0; i < static_cast<int>(groupes.size()); ++i)
    {
        const GroupeConsommable& groupe = groupes[i];

        std::cout << "[" << i << "] " << groupe.nom
                  << " x" << groupe.quantite
                  << " | " << InventaireUtils::typeConsommableVersTexte(groupe.type)
                  << " | Puissance : " << groupe.puissance
                  << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Matériaux : pas encore disponibles" << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << std::endl;
}

void InventaireAffichage::afficherArmeSelectionnee(const Arme& arme)
{
    std::cout << "========== ARME SÉLECTIONNÉE ==========" << std::endl;
    std::cout << "Arme : " << arme.getNom() << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;
    std::cout << "2 : Équiper" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void InventaireAffichage::afficherArmureSelectionnee(const Armure& armure)
{
    std::cout << "========== ARMURE SÉLECTIONNÉE ==========" << std::endl;
    std::cout << "Armure : " << armure.getNom() << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;
    std::cout << "2 : Équiper" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void InventaireAffichage::afficherConsommableSelectionne(const Consommable& consommable)
{
    std::cout << "========== CONSOMMABLE SÉLECTIONNÉ ==========" << std::endl;
    std::cout << "Consommable : " << consommable.getNom() << std::endl;
    std::cout << "Type : " << InventaireUtils::typeConsommableVersTexte(consommable.getType()) << std::endl;
    std::cout << "Puissance : " << consommable.getPuissance() << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;

    if (consommable.getType() == TypeConsommable::Soin)
    {
        std::cout << "2 : Utiliser" << std::endl;
    }
    else
    {
        std::cout << "2 : Utiliser (à faire depuis le menu Potions en combat)" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "> ";
}

void InventaireAffichage::afficherMateriauxIndisponibles()
{
    std::cout << "========== MATÉRIAUX ==========" << std::endl;
    std::cout << "Les matériaux ne sont pas encore disponibles." << std::endl;
    std::cout << "Plus tard, ils serviront à réparer, améliorer et fabriquer de l'équipement." << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << std::endl;
}