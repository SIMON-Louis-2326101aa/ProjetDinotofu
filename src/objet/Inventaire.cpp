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

void Inventaire::afficherListeArmes() const
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
        std::cout << "[" << i << "] " << armes[i].getNom();

        if (!armes[i].estIndestructible())
        {
            std::cout << " (" << armes[i].getDurabilite() << "/" << armes[i].getDurabiliteMax() << ")";
        }

        if (armes[i].estCassee())
        {
            std::cout << " - Cassée";
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
}

void Inventaire::afficherListeArmures() const
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
        std::cout << "[" << i << "] " << armures[i].getNom();

        if (!armures[i].estIndestructible())
        {
            std::cout << " (" << armures[i].getDurabilite() << "/" << armures[i].getDurabiliteMax() << ")";
        }

        if (armures[i].estCassee())
        {
            std::cout << " - Cassée";
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
}

void Inventaire::afficherListeConsommables() const
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
    }

    std::cout << std::endl;
}

void Inventaire::afficherResume() const
{
    std::cout << "================ INVENTAIRE ================" << std::endl;
    std::cout << "Or : " << or_ << " pièces" << std::endl;
    std::cout << "Armes : " << getNombreArmes() << std::endl;
    std::cout << "Armures : " << getNombreArmures() << std::endl;
    std::cout << "Consommables : " << getNombreConsommables() << std::endl;
    std::cout << "Potions de soin : " << compterConsommables(TypeConsommable::Soin) << std::endl;
    std::cout << "Potions de rage : " << compterConsommables(TypeConsommable::Degats) << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << std::endl;
}

void Inventaire::inspecterArme(int index) const
{
    if (!possedeArme(index))
    {
        std::cout << "Cette arme n'existe pas." << std::endl;
        std::cout << std::endl;
        return;
    }

    const Arme& arme = armes[index];

    std::cout << "========== ARME ==========" << std::endl;
    std::cout << "Nom : " << arme.getNom() << std::endl;
    std::cout << "Description : " << arme.getDescription() << std::endl;
    std::cout << "Bonus dégâts : +" << arme.getBonusDegatsMin()
              << " à +" << arme.getBonusDegatsMax() << std::endl;
    std::cout << "Bonus critique : +" << arme.getBonusCritique() << std::endl;

    if (arme.estIndestructible())
    {
        std::cout << "Durabilité : Indestructible" << std::endl;
    }
    else
    {
        std::cout << "Durabilité : " << arme.getDurabilite()
                  << "/" << arme.getDurabiliteMax() << std::endl;
    }

    std::cout << "État : " << (arme.estCassee() ? "Cassée" : "Utilisable") << std::endl;
    std::cout << "Valeur : " << arme.getValeur() << " pièces" << std::endl;
    std::cout << "==========================" << std::endl;
    std::cout << std::endl;
}

void Inventaire::inspecterArmure(int index) const
{
    if (!possedeArmure(index))
    {
        std::cout << "Cette armure n'existe pas." << std::endl;
        std::cout << std::endl;
        return;
    }

    const Armure& armure = armures[index];

    std::cout << "========== ARMURE ==========" << std::endl;
    std::cout << "Nom : " << armure.getNom() << std::endl;
    std::cout << "Description : " << armure.getDescription() << std::endl;
    std::cout << "Bonus PV max : +" << armure.getBonusPvMax() << std::endl;
    std::cout << "Réduction dégâts : " << armure.getReductionDegats() << std::endl;

    if (armure.estIndestructible())
    {
        std::cout << "Durabilité : Indestructible" << std::endl;
    }
    else
    {
        std::cout << "Durabilité : " << armure.getDurabilite()
                  << "/" << armure.getDurabiliteMax() << std::endl;
    }

    std::cout << "État : " << (armure.estCassee() ? "Cassée" : "Utilisable") << std::endl;
    std::cout << "Valeur : " << armure.getValeur() << " pièces" << std::endl;
    std::cout << "============================" << std::endl;
    std::cout << std::endl;
}

void Inventaire::inspecterConsommable(int index) const
{
    if (!possedeConsommable(index))
    {
        std::cout << "Ce consommable n'existe pas." << std::endl;
        std::cout << std::endl;
        return;
    }

    const Consommable& consommable = consommables[index];

    std::cout << "======= CONSOMMABLE =======" << std::endl;
    std::cout << "Nom : " << consommable.getNom() << std::endl;
    std::cout << "Description : " << consommable.getDescription() << std::endl;
    std::cout << "Puissance : " << consommable.getPuissance() << std::endl;
    std::cout << "Valeur : " << consommable.getValeur() << " pièces" << std::endl;
    std::cout << "===========================" << std::endl;
    std::cout << std::endl;
}

void Inventaire::afficherArmes() const
{
    afficherListeArmes();
}

void Inventaire::afficherArmures() const
{
    afficherListeArmures();
}

void Inventaire::afficherConsommables() const
{
    afficherListeConsommables();
}

void Inventaire::afficher() const
{
    afficherResume();

    afficherListeArmes();
    afficherListeArmures();
    afficherListeConsommables();
}