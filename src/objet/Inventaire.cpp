#include "objet/Inventaire.hpp"

#include <iostream>

Inventaire::Inventaire()
{
    or_ = 0;
}

int Inventaire::getOr() const
{
    return or_;
}

void Inventaire::gagnerOr(int quantite)
{
    if (quantite <= 0)
    {
        return;
    }

    or_ += quantite;
}

bool Inventaire::depenserOr(int quantite)
{
    if (quantite <= 0)
    {
        return true;
    }

    if (or_ < quantite)
    {
        return false;
    }

    or_ -= quantite;
    return true;
}

int Inventaire::getNombreArmes() const
{
    return static_cast<int>(armes.size());
}

int Inventaire::getNombreConsommables() const
{
    return static_cast<int>(consommables.size());
}

const std::vector<Arme>& Inventaire::getArmes() const
{
    return armes;
}

const std::vector<Consommable>& Inventaire::getConsommables() const
{
    return consommables;
}

void Inventaire::ajouterArme(const Arme& arme)
{
    armes.push_back(arme);
}

void Inventaire::ajouterConsommable(const Consommable& consommable)
{
    consommables.push_back(consommable);
}

bool Inventaire::possedeArme(int index) const
{
    return index >= 0 && index < static_cast<int>(armes.size());
}

bool Inventaire::possedeConsommable(int index) const
{
    return index >= 0 && index < static_cast<int>(consommables.size());
}

Arme Inventaire::getArme(int index) const
{
    if (!possedeArme(index))
    {
        return Arme();
    }

    return armes[index];
}

Arme* Inventaire::getArmeModifiable(int index)
{
    if (!possedeArme(index))
    {
        return nullptr;
    }

    return &armes[index];
}

Consommable Inventaire::getConsommable(int index) const
{
    if (!possedeConsommable(index))
    {
        return Consommable();
    }

    return consommables[index];
}

bool Inventaire::retirerConsommable(int index)
{
    if (!possedeConsommable(index))
    {
        return false;
    }

    consommables.erase(consommables.begin() + index);
    return true;
}

void Inventaire::afficherArmes() const
{
    std::cout << "===== ARMES =====" << std::endl;

    if (armes.empty())
    {
        std::cout << "Aucune arme dans l'inventaire." << std::endl;
        std::cout << std::endl;
        return;
    }

    for (int i = 0; i < static_cast<int>(armes.size()); i++)
    {
        std::cout << "[" << i << "] " << armes[i].getNom() << std::endl;
        std::cout << "    " << armes[i].getDescription() << std::endl;
        std::cout << "    Bonus dégâts : +"
                  << armes[i].getBonusDegatsMin()
                  << " à +"
                  << armes[i].getBonusDegatsMax()
                  << std::endl;
        std::cout << "    Bonus critique : +"
                  << armes[i].getBonusCritique()
                  << std::endl;

        if (armes[i].estIndestructible())
        {
            std::cout << "    Durabilité : Indestructible" << std::endl;
        }
        else
        {
            std::cout << "    Durabilité : "
                      << armes[i].getDurabilite()
                      << "/"
                      << armes[i].getDurabiliteMax()
                      << std::endl;

            if (armes[i].estCassee())
            {
                std::cout << "    État : Cassée, ses bonus ne s'appliquent plus." << std::endl;
            }
        }

        std::cout << "    Valeur : " << armes[i].getValeur() << " pièces" << std::endl;
        std::cout << std::endl;
    }
}

void Inventaire::afficherConsommables() const
{
    std::cout << "===== CONSOMMABLES =====" << std::endl;

    if (consommables.empty())
    {
        std::cout << "Aucun consommable dans l'inventaire." << std::endl;
        std::cout << std::endl;
        return;
    }

    for (int i = 0; i < static_cast<int>(consommables.size()); i++)
    {
        std::cout << "[" << i << "] " << consommables[i].getNom() << std::endl;
        std::cout << "    " << consommables[i].getDescription() << std::endl;
        std::cout << "    Puissance : " << consommables[i].getPuissance() << std::endl;
        std::cout << "    Valeur : " << consommables[i].getValeur() << " pièces" << std::endl;
        std::cout << std::endl;
    }
}

void Inventaire::afficher() const
{
    std::cout << "================ INVENTAIRE ================" << std::endl;
    std::cout << "Or : " << or_ << " pièces" << std::endl;
    std::cout << std::endl;

    afficherArmes();
    afficherConsommables();

    std::cout << "============================================" << std::endl;
    std::cout << std::endl;
}