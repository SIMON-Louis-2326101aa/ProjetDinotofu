#include "interface/menu/potions/PotionsCombatAffichage.hpp"

#include "interface/menu/potions/PotionsCombatUtils.hpp"

#include "objet/Inventaire.hpp"
#include "objet/consommable/Consommable.hpp"

#include <iostream>

void PotionsCombatAffichage::afficherMenuPrincipal()
{
    std::cout << "========== POTIONS ==========" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Voir les potions" << std::endl;
    std::cout << "2 : Utiliser une potion curative" << std::endl;
    std::cout << "3 : Utiliser une potion défensive" << std::endl;
    std::cout << "4 : Utiliser une potion offensive" << std::endl;
    std::cout << "5 : Utiliser une potion de buff" << std::endl;
    std::cout << "6 : Utiliser une potion de debuff" << std::endl;
    std::cout << "============================" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void PotionsCombatAffichage::afficherSoinRapide(
    const Joueur& joueur,
    const std::vector<int>& indices
)
{
    std::cout << "========== POTION DE SOIN RAPIDE ==========" << std::endl;

    for (int i = 0; i < static_cast<int>(indices.size()); ++i)
    {
        int indexInventaire = indices[i];
        Consommable potion = joueur.getInventaire().getConsommable(indexInventaire);

        std::cout << i + 1
                  << " : "
                  << potion.getNom()
                  << " | Soin : "
                  << potion.getPuissance()
                  << " PV"
                  << std::endl;
    }

    std::cout << "===========================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void PotionsCombatAffichage::afficherPotionSoinSelectionnee(const Consommable& potion)
{
    std::cout << "========== POTION SÉLECTIONNÉE ==========" << std::endl;
    std::cout << "Potion : " << potion.getNom() << std::endl;
    std::cout << "Description : " << potion.getDescription() << std::endl;
    std::cout << "Soin : " << potion.getPuissance() << " PV" << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;
    std::cout << "2 : Utiliser" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void PotionsCombatAffichage::afficherPotionSelectionnee(const Consommable& potion)
{
    std::cout << "========== POTION SÉLECTIONNÉE ==========" << std::endl;
    std::cout << "Potion : " << potion.getNom() << std::endl;
    std::cout << "Type : " << PotionsCombatUtils::typeVersTexte(potion.getType()) << std::endl;
    std::cout << "Puissance : " << potion.getPuissance() << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;
    std::cout << "2 : Utiliser" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void PotionsCombatAffichage::afficherPotions(const Joueur& joueur)
{
    const std::vector<Consommable>& consommables =
        joueur.getInventaire().getConsommables();

    std::cout << "========== POTIONS DISPONIBLES ==========" << std::endl;

    if (consommables.empty())
    {
        std::cout << "Aucune potion dans l'inventaire." << std::endl;
        std::cout << "=========================================" << std::endl;
        std::cout << std::endl;
        return;
    }

    for (int i = 0; i < static_cast<int>(consommables.size()); ++i)
    {
        const Consommable& potion = consommables[i];

        std::cout << "[" << i << "] "
                  << potion.getNom()
                  << " | "
                  << PotionsCombatUtils::typeVersTexte(potion.getType())
                  << " | Puissance : "
                  << potion.getPuissance()
                  << std::endl;
    }

    std::cout << "=========================================" << std::endl;
    std::cout << std::endl;
}

void PotionsCombatAffichage::afficherPotionsFiltrees(
    const Joueur& joueur,
    const std::vector<int>& indices
)
{
    std::cout << "========== LISTE DES POTIONS ==========" << std::endl;

    for (int i = 0; i < static_cast<int>(indices.size()); ++i)
    {
        int indexInventaire = indices[i];
        Consommable potion = joueur.getInventaire().getConsommable(indexInventaire);

        std::cout << i + 1
                  << " : "
                  << potion.getNom()
                  << " | Puissance : "
                  << potion.getPuissance()
                  << std::endl;
    }

    std::cout << "=======================================" << std::endl;
    std::cout << "Choisis une potion." << std::endl;
    std::cout << "Entre son numéro dans la liste, ou 0 pour revenir." << std::endl;
    std::cout << "> ";
}