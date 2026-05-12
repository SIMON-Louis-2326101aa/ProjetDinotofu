#include "objet/armure/Armure.hpp"

#include <iostream>

Armure::Armure() : Objet()
{
    type = TypeArmure::Inconnue;

    bonusPvMax = 0;
    reductionDegats = 0;

    durabiliteMax = -1;
    durabilite = -1;
}

Armure::Armure(
    const std::string& nom,
    const std::string& description,
    int valeur,
    TypeArmure type,
    int bonusPvMax,
    int reductionDegats,
    int durabiliteMax
) : Objet(nom, description, valeur)
{
    this->type = type;

    this->bonusPvMax = bonusPvMax;
    this->reductionDegats = reductionDegats;

    this->durabiliteMax = durabiliteMax;
    this->durabilite = durabiliteMax;
}

TypeArmure Armure::getType() const
{
    return type;
}

int Armure::getBonusPvMax() const
{
    return bonusPvMax;
}

int Armure::getReductionDegats() const
{
    return reductionDegats;
}

int Armure::getDurabilite() const
{
    return durabilite;
}

int Armure::getDurabiliteMax() const
{
    return durabiliteMax;
}

bool Armure::estCassee() const
{
    if (estIndestructible())
    {
        return false;
    }

    return durabilite <= 0;
}

bool Armure::estIndestructible() const
{
    return durabiliteMax < 0;
}

void Armure::perdreDurabilite(int quantite)
{
    if (estIndestructible() || quantite <= 0)
    {
        return;
    }

    durabilite -= quantite;

    if (durabilite < 0)
    {
        durabilite = 0;
    }
}

void Armure::reparer(int quantite)
{
    if (estIndestructible() || quantite <= 0)
    {
        return;
    }

    durabilite += quantite;

    if (durabilite > durabiliteMax)
    {
        durabilite = durabiliteMax;
    }
}

void Armure::reparerCompletement()
{
    if (estIndestructible())
    {
        return;
    }

    durabilite = durabiliteMax;
}

void Armure::afficher() const
{
    std::cout << "===== ARMURE =====" << std::endl;
    std::cout << "Nom : " << nom << std::endl;
    std::cout << "Description : " << description << std::endl;
    std::cout << "Valeur : " << valeur << " pièces" << std::endl;

    std::cout << "Type : ";

    switch (type)
    {
        case TypeArmure::Tissu:
            std::cout << "Tissu";
            break;

        case TypeArmure::Cuir:
            std::cout << "Cuir";
            break;

        case TypeArmure::Maille:
            std::cout << "Maille";
            break;

        case TypeArmure::Plaque:
            std::cout << "Plaque";
            break;

        case TypeArmure::Magique:
            std::cout << "Magique";
            break;

        default:
            std::cout << "Inconnue";
            break;
    }

    std::cout << std::endl;
    std::cout << "Bonus PV max : " << bonusPvMax << std::endl;
    std::cout << "Réduction dégâts : " << reductionDegats << std::endl;

    if (estIndestructible())
    {
        std::cout << "Durabilité : Indestructible" << std::endl;
    }
    else
    {
        std::cout << "Durabilité : " << durabilite << "/" << durabiliteMax << std::endl;

        if (estCassee())
        {
            std::cout << "État : Cassée, ses bonus ne s'appliquent plus." << std::endl;
        }
    }

    std::cout << "==================" << std::endl;
    std::cout << std::endl;
}