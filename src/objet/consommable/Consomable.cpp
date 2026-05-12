#include "objet/consommable/Consommable.hpp"

#include <iostream>

Consommable::Consommable() : Objet()
{
    type = TypeConsommable::Inconnu;
    puissance = 0;
}

Consommable::Consommable(
    const std::string& nom,
    const std::string& description,
    int valeur,
    TypeConsommable type,
    int puissance
) : Objet(nom, description, valeur)
{
    this->type = type;
    this->puissance = puissance;
}

TypeConsommable Consommable::getType() const
{
    return type;
}

int Consommable::getPuissance() const
{
    return puissance;
}

bool Consommable::estSoin() const
{
    return type == TypeConsommable::Soin;
}

bool Consommable::estDegats() const
{
    return type == TypeConsommable::Degats;
}

void Consommable::afficher() const
{
    std::cout << "===== CONSOMMABLE =====" << std::endl;
    std::cout << "Nom : " << nom << std::endl;
    std::cout << "Description : " << description << std::endl;
    std::cout << "Valeur : " << valeur << " pièces" << std::endl;

    std::cout << "Type : ";

    switch (type)
    {
        case TypeConsommable::Soin:
            std::cout << "Soin";
            break;

        case TypeConsommable::Degats:
            std::cout << "Dégâts";
            break;

        case TypeConsommable::Buff:
            std::cout << "Buff";
            break;

        case TypeConsommable::Debuff:
            std::cout << "Debuff";
            break;

        case TypeConsommable::Special:
            std::cout << "Spécial";
            break;

        default:
            std::cout << "Inconnu";
            break;
    }

    std::cout << std::endl;
    std::cout << "Puissance : " << puissance << std::endl;
    std::cout << "=======================" << std::endl;
    std::cout << std::endl;
}