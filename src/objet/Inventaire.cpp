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

int Inventaire::getNombreArmures() const
{
    return static_cast<int>(armures.size());
}

int Inventaire::getNombreConsommables() const
{
    return static_cast<int>(consommables.size());
}

int Inventaire::compterConsommables(TypeConsommable type) const
{
    int total = 0;

    for (const Consommable& consommable : consommables)
    {
        if (consommable.getType() == type)
        {
            total++;
        }
    }

    return total;
}

const std::vector<Arme>& Inventaire::getArmes() const
{
    return armes;
}

const std::vector<Armure>& Inventaire::getArmures() const
{
    return armures;
}

const std::vector<Consommable>& Inventaire::getConsommables() const
{
    return consommables;
}

void Inventaire::ajouterArme(const Arme& arme)
{
    armes.push_back(arme);
}

void Inventaire::ajouterArmure(const Armure& armure)
{
    armures.push_back(armure);
}

void Inventaire::ajouterConsommable(const Consommable& consommable)
{
    consommables.push_back(consommable);
}

bool Inventaire::possedeArme(int index) const
{
    return index >= 0 && index < static_cast<int>(armes.size());
}

bool Inventaire::possedeArmure(int index) const
{
    return index >= 0 && index < static_cast<int>(armures.size());
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

Armure Inventaire::getArmure(int index) const
{
    if (!possedeArmure(index))
    {
        return Armure();
    }

    return armures[index];
}

Armure* Inventaire::getArmureModifiable(int index)
{
    if (!possedeArmure(index))
    {
        return nullptr;
    }

    return &armures[index];
}

Consommable Inventaire::getConsommable(int index) const
{
    if (!possedeConsommable(index))
    {
        return Consommable();
    }

    return consommables[index];
}

int Inventaire::trouverPremierConsommable(TypeConsommable type) const
{
    for (int i = 0; i < static_cast<int>(consommables.size()); i++)
    {
        if (consommables[i].getType() == type)
        {
            return i;
        }
    }

    return -1;
}

bool Inventaire::utiliserPremierConsommable(TypeConsommable type, Consommable& consommableUtilise)
{
    int index = trouverPremierConsommable(type);

    if (index == -1)
    {
        return false;
    }

    consommableUtilise = consommables[index];
    consommables.erase(consommables.begin() + index);

    return true;
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

void Inventaire::afficherArmures() const
{
    std::cout << "===== ARMURES =====" << std::endl;

    if (armures.empty())
    {
        std::cout << "Aucune armure dans l'inventaire." << std::endl;
        std::cout << std::endl;
        return;
    }

    for (int i = 0; i < static_cast<int>(armures.size()); i++)
    {
        std::cout << "[" << i << "] " << armures[i].getNom() << std::endl;
        std::cout << "    " << armures[i].getDescription() << std::endl;
        std::cout << "    Bonus PV max : +" << armures[i].getBonusPvMax() << std::endl;
        std::cout << "    Réduction dégâts : " << armures[i].getReductionDegats() << std::endl;

        if (armures[i].estIndestructible())
        {
            std::cout << "    Durabilité : Indestructible" << std::endl;
        }
        else
        {
            std::cout << "    Durabilité : "
                      << armures[i].getDurabilite()
                      << "/"
                      << armures[i].getDurabiliteMax()
                      << std::endl;

            if (armures[i].estCassee())
            {
                std::cout << "    État : Cassée, ses bonus ne s'appliquent plus." << std::endl;
            }
        }

        std::cout << "    Valeur : " << armures[i].getValeur() << " pièces" << std::endl;
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
    afficherArmures();
    afficherConsommables();

    std::cout << "============================================" << std::endl;
    std::cout << std::endl;
}