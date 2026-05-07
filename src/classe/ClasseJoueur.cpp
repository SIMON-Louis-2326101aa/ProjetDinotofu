#include "classe/ClasseJoueur.hpp"

ClasseJoueur::ClasseJoueur()
{
    nom = "Aucune";
    pvMax = 100;
    degatsMin = 1;
    degatsMax = 5;
    degatsCrit = 10;
    potionsSoin = 0;
    potionsDegats = 0;
}

ClasseJoueur::ClasseJoueur(
    const std::string& nom,
    int pvMax,
    int degatsMin,
    int degatsMax,
    int degatsCrit,
    int potionsSoin,
    int potionsDegats
)
{
    this->nom = nom;
    this->pvMax = pvMax;
    this->degatsMin = degatsMin;
    this->degatsMax = degatsMax;
    this->degatsCrit = degatsCrit;
    this->potionsSoin = potionsSoin;
    this->potionsDegats = potionsDegats;
}

std::string ClasseJoueur::getNom() const
{
    return nom;
}

int ClasseJoueur::getPvMax() const
{
    return pvMax;
}

int ClasseJoueur::getDegatsMin() const
{
    return degatsMin;
}

int ClasseJoueur::getDegatsMax() const
{
    return degatsMax;
}

int ClasseJoueur::getDegatsCrit() const
{
    return degatsCrit;
}

int ClasseJoueur::getPotionsSoin() const
{
    return potionsSoin;
}

int ClasseJoueur::getPotionsDegats() const
{
    return potionsDegats;
}