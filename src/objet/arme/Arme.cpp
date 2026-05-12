#include "objet/arme/Arme.hpp"

#include <iostream>

Arme::Arme() : Objet()
{
    type = TypeArme::MainsNues;

    bonusDegatsMin = 0;
    bonusDegatsMax = 0;
    bonusCritique = 0;

    durabiliteMax = -1;
    durabilite = -1;
}

Arme::Arme(
    const std::string& nom,
    const std::string& description,
    int valeur,
    TypeArme type,
    int bonusDegatsMin,
    int bonusDegatsMax,
    int bonusCritique,
    int durabiliteMax
) : Objet(nom, description, valeur)
{
    this->type = type;

    this->bonusDegatsMin = bonusDegatsMin;
    this->bonusDegatsMax = bonusDegatsMax;
    this->bonusCritique = bonusCritique;

    this->durabiliteMax = durabiliteMax;
    this->durabilite = durabiliteMax;
}

TypeArme Arme::getType() const
{
    return type;
}

int Arme::getBonusDegatsMin() const
{
    return bonusDegatsMin;
}

int Arme::getBonusDegatsMax() const
{
    return bonusDegatsMax;
}

int Arme::getBonusCritique() const
{
    return bonusCritique;
}

int Arme::getDurabilite() const
{
    return durabilite;
}

int Arme::getDurabiliteMax() const
{
    return durabiliteMax;
}

bool Arme::estCassee() const
{
    if (estIndestructible())
    {
        return false;
    }

    return durabilite <= 0;
}

bool Arme::estIndestructible() const
{
    return durabiliteMax < 0;
}

void Arme::perdreDurabilite(int quantite)
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

void Arme::reparer(int quantite)
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

void Arme::reparerCompletement()
{
    if (estIndestructible())
    {
        return;
    }

    durabilite = durabiliteMax;
}

void Arme::afficher() const
{
    std::cout << "===== ARME =====" << std::endl;
    std::cout << "Nom : " << nom << std::endl;
    std::cout << "Description : " << description << std::endl;
    std::cout << "Valeur : " << valeur << " pièces" << std::endl;

    std::cout << "Type : ";

    switch (type)
    {
        case TypeArme::Epee:
            std::cout << "Épée";
            break;

        case TypeArme::Dague:
            std::cout << "Dague";
            break;

        case TypeArme::Hache:
            std::cout << "Hache";
            break;

        case TypeArme::Marteau:
            std::cout << "Marteau";
            break;

        case TypeArme::Lance:
            std::cout << "Lance";
            break;

        case TypeArme::Baton:
            std::cout << "Bâton";
            break;

        case TypeArme::Arc:
            std::cout << "Arc";
            break;

        case TypeArme::MainsNues:
            std::cout << "Mains nues";
            break;

        default:
            std::cout << "Inconnu";
            break;
    }

    std::cout << std::endl;
    std::cout << "Bonus dégâts min : " << bonusDegatsMin << std::endl;
    std::cout << "Bonus dégâts max : " << bonusDegatsMax << std::endl;
    std::cout << "Bonus critique : " << bonusCritique << std::endl;

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

    std::cout << "================" << std::endl;
    std::cout << std::endl;
}