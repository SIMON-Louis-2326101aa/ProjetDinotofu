#include "entite/Joueur.hpp"

#include <iostream>

Joueur::Joueur() : Entite()
{
    niveau = 1;
    experience = 0;
    argent = 0;
}

Joueur::Joueur(
    const std::string& nom,
    const ClasseJoueur& classeChoisie
) : Entite(
        nom,
        classeChoisie.getNom(),
        classeChoisie.getPvMax(),
        classeChoisie.getDegatsMin(),
        classeChoisie.getDegatsMax(),
        classeChoisie.getDegatsCrit(),
        classeChoisie.getPotionsSoin(),
        classeChoisie.getPotionsDegats()
    )
{
    niveau = 1;
    experience = 0;
    argent = 0;
}

int Joueur::getNiveau() const
{
    return niveau;
}

int Joueur::getExperience() const
{
    return experience;
}

int Joueur::getArgent() const
{
    return argent;
}

void Joueur::afficherStats() const
{
    std::cout << "===== FICHE DU JOUEUR =====" << std::endl;
    std::cout << "Nom : " << nom << std::endl;
    std::cout << "Classe : " << type << std::endl;
    std::cout << "Niveau : " << niveau << std::endl;
    std::cout << "Expérience : " << experience << std::endl;
    std::cout << "Argent : " << argent << std::endl;
    std::cout << "PV : " << pv << "/" << pvMax << std::endl;
    std::cout << "Dégâts : " << degatsMin << " - " << degatsMax << std::endl;
    std::cout << "Critique : " << degatsCrit << std::endl;
    std::cout << "Potions de soin : " << potionsSoin << std::endl;
    std::cout << "Potions de dégâts : " << potionsDegats << std::endl;
    std::cout << "===========================" << std::endl;
    std::cout << std::endl;
}