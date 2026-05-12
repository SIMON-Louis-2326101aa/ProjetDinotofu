#include "objet/Objet.hpp"

#include <iostream>

Objet::Objet()
{
    nom = "Objet inconnu";
    description = "Un objet mystérieux dont l'utilité reste floue.";
    valeur = 0;
}

Objet::Objet(
    const std::string& nom,
    const std::string& description,
    int valeur
)
{
    this->nom = nom;
    this->description = description;
    this->valeur = valeur;
}

std::string Objet::getNom() const
{
    return nom;
}

std::string Objet::getDescription() const
{
    return description;
}

int Objet::getValeur() const
{
    return valeur;
}

void Objet::afficher() const
{
    std::cout << "===== OBJET =====" << std::endl;
    std::cout << "Nom : " << nom << std::endl;
    std::cout << "Description : " << description << std::endl;
    std::cout << "Valeur : " << valeur << " pièces" << std::endl;
    std::cout << "=================" << std::endl;
    std::cout << std::endl;
}