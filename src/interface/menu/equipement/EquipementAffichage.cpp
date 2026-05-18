#include "interface/menu/equipement/EquipementAffichage.hpp"

#include <iostream>
#include <string>

void EquipementAffichage::afficherMenuPrincipal()
{
    std::cout << "========== ÉQUIPEMENT ==========" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Voir équipement simple" << std::endl;
    std::cout << "2 : Voir équipement détaillé" << std::endl;
    std::cout << "3 : Changer arme rapide" << std::endl;
    std::cout << "4 : Changer tenue rapide" << std::endl;
    std::cout << "================================" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void EquipementAffichage::afficherListeArmes(const Joueur& joueur)
{
    std::cout << "========== CHANGER ARME ==========" << std::endl;

    for (int i = 0; i < joueur.getInventaire().getNombreArmes(); ++i)
    {
        Arme arme = joueur.getInventaire().getArme(i);
        afficherResumeArme(arme, i);
    }

    std::cout << "==================================" << std::endl;
    std::cout << "Choisis l'arme à équiper." << std::endl;
    std::cout << "Entre son numéro, ou -1 pour revenir." << std::endl;
    std::cout << "> ";
}

void EquipementAffichage::afficherListeArmures(const Joueur& joueur)
{
    std::cout << "========== CHANGER TENUE ==========" << std::endl;

    for (int i = 0; i < joueur.getInventaire().getNombreArmures(); ++i)
    {
        Armure armure = joueur.getInventaire().getArmure(i);
        afficherResumeArmure(armure, i);
    }

    std::cout << "===================================" << std::endl;
    std::cout << "Choisis l'armure à équiper." << std::endl;
    std::cout << "Entre son numéro, ou -1 pour revenir." << std::endl;
    std::cout << "> ";
}

void EquipementAffichage::afficherArmeSelectionnee(const Arme& arme)
{
    std::cout << "========== ARME SÉLECTIONNÉE ==========" << std::endl;
    std::cout << "Arme : " << arme.getNom() << std::endl;
    std::cout << "Durabilité : " << durabiliteArmeTexte(arme) << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;
    std::cout << "2 : Comparer" << std::endl;
    std::cout << "3 : Équiper" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void EquipementAffichage::afficherArmureSelectionnee(const Armure& armure)
{
    std::cout << "========== ARMURE SÉLECTIONNÉE ==========" << std::endl;
    std::cout << "Armure : " << armure.getNom() << std::endl;
    std::cout << "Durabilité : " << durabiliteArmureTexte(armure) << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;
    std::cout << "2 : Comparer" << std::endl;
    std::cout << "3 : Équiper" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void EquipementAffichage::afficherResumeArme(const Arme& arme, int index)
{
    std::cout << index << " : " << arme.getNom()
              << " | Dégâts : +" << arme.getBonusDegatsMin()
              << " à +" << arme.getBonusDegatsMax()
              << " | Critique : +" << arme.getBonusCritique()
              << " | Durabilité : " << durabiliteArmeTexte(arme);

    if (arme.estCassee())
    {
        std::cout << " | Cassée";
    }

    std::cout << std::endl;
}

void EquipementAffichage::afficherResumeArmure(const Armure& armure, int index)
{
    std::cout << index << " : " << armure.getNom()
              << " | PV max : +" << armure.getBonusPvMax()
              << " | Réduction : " << armure.getReductionDegats()
              << " | Durabilité : " << durabiliteArmureTexte(armure);

    if (armure.estCassee())
    {
        std::cout << " | Cassée";
    }

    std::cout << std::endl;
}

std::string EquipementAffichage::durabiliteArmeTexte(const Arme& arme)
{
    if (arme.estIndestructible())
    {
        return "indestructible";
    }

    return std::to_string(arme.getDurabilite()) + "/" + std::to_string(arme.getDurabiliteMax());
}

std::string EquipementAffichage::durabiliteArmureTexte(const Armure& armure)
{
    if (armure.estIndestructible())
    {
        return "indestructible";
    }

    return std::to_string(armure.getDurabilite()) + "/" + std::to_string(armure.getDurabiliteMax());
}