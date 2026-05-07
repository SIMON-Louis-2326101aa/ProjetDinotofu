#include "entite/Entite.hpp"

#include <iostream>

Entite::Entite()
{
    nom = "Inconnu";
    type = "Aucun";

    pv = 100;
    pvMax = 100;

    degatsMin = 1;
    degatsMax = 5;
    degatsCrit = 10;

    potionsSoin = 0;
    potionsDegats = 0;
}

Entite::Entite(
    const std::string& nom,
    const std::string& type,
    int pvMax,
    int degatsMin,
    int degatsMax,
    int degatsCrit,
    int potionsSoin,
    int potionsDegats
)
{
    this->nom = nom;
    this->type = type;

    this->pv = pvMax;
    this->pvMax = pvMax;

    this->degatsMin = degatsMin;
    this->degatsMax = degatsMax;
    this->degatsCrit = degatsCrit;

    this->potionsSoin = potionsSoin;
    this->potionsDegats = potionsDegats;
}

std::string Entite::getNom() const
{
    return nom;
}

std::string Entite::getType() const
{
    return type;
}

int Entite::getPv() const
{
    return pv;
}

int Entite::getPvMax() const
{
    return pvMax;
}

int Entite::getDegatsMin() const
{
    return degatsMin;
}

int Entite::getDegatsMax() const
{
    return degatsMax;
}

int Entite::getDegatsCrit() const
{
    return degatsCrit;
}

int Entite::getPotionsSoin() const
{
    return potionsSoin;
}

int Entite::getPotionsDegats() const
{
    return potionsDegats;
}

bool Entite::estMort() const
{
    return pv <= 0;
}

void Entite::recevoirDegats(int degats)
{
    if (degats < 0)
    {
        return;
    }

    pv -= degats;

    if (pv < 0)
    {
        pv = 0;
    }
}

void Entite::soigner(int valeurSoin)
{
    if (valeurSoin <= 0)
    {
        return;
    }

    pv += valeurSoin;

    if (pv > pvMax)
    {
        pv = pvMax;
    }
}

void Entite::reduirePvMax(int valeur)
{
    if (valeur <= 0)
    {
        return;
    }

    pvMax -= valeur;

    if (pvMax < 1)
    {
        pvMax = 1;
    }

    if (pv > pvMax)
    {
        pv = pvMax;
    }
}

int Entite::attaquer(Random& random, bool& esquive, bool& critique, int bonusDegats)
{
    int resultat = random.lancerD20();

    esquive = false;
    critique = false;

    if (resultat <= 3)
    {
        esquive = true;
        return 0;
    }

    if (resultat <= 16)
    {
        return random.entre(degatsMin, degatsMax) + bonusDegats;
    }

    critique = true;
    return degatsCrit + bonusDegats;
}

bool Entite::utiliserPotionSoin(int valeurSoin)
{
    if (potionsSoin <= 0)
    {
        return false;
    }

    potionsSoin--;
    soigner(valeurSoin);

    return true;
}

bool Entite::consommerPotionDegats()
{
    if (potionsDegats <= 0)
    {
        return false;
    }

    potionsDegats--;
    return true;
}

void Entite::appliquerClasse(const ClasseJoueur& nouvelleClasse)
{
    type = nouvelleClasse.getNom();

    pvMax = nouvelleClasse.getPvMax();
    pv = pvMax;

    degatsMin = nouvelleClasse.getDegatsMin();
    degatsMax = nouvelleClasse.getDegatsMax();
    degatsCrit = nouvelleClasse.getDegatsCrit();

    potionsSoin = nouvelleClasse.getPotionsSoin();
    potionsDegats = nouvelleClasse.getPotionsDegats();
}

bool Entite::statsVisibles() const
{
    return true;
}

void Entite::afficherStats() const
{
    std::cout << nom << std::endl;
    std::cout << "Type : " << type << std::endl;
    std::cout << "PV : " << pv << "/" << pvMax << std::endl;
    std::cout << "Dégâts min : " << degatsMin << std::endl;
    std::cout << "Dégâts max : " << degatsMax << std::endl;
    std::cout << "Dégâts crit : " << degatsCrit << std::endl;
    std::cout << "Potions de soin : " << potionsSoin << std::endl;
    std::cout << "Potions de dégâts : " << potionsDegats << std::endl;
    std::cout << std::endl;
}