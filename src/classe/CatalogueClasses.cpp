#include "classe/CatalogueClasses.hpp"

#include <iostream>

void CatalogueClasses::afficherClassesDeBase()
{
    std::cout << "1 : Chevalier" << std::endl;
    std::cout << "    PV : 200" << std::endl;
    std::cout << "    Dégâts : 5 - 20 | Critique : 30" << std::endl;
    std::cout << "    Potions de soin : 4" << std::endl;
    std::cout << "    Potions de dégâts : 3" << std::endl;
    std::cout << std::endl;

    std::cout << "2 : Classe Légère" << std::endl;
    std::cout << "    PV : 150" << std::endl;
    std::cout << "    Dégâts : 10 - 30 | Critique : 45" << std::endl;
    std::cout << "    Potions de soin : 5" << std::endl;
    std::cout << "    Potions de dégâts : 2" << std::endl;
    std::cout << std::endl;

    std::cout << "3 : Classe Lourde" << std::endl;
    std::cout << "    PV : 400" << std::endl;
    std::cout << "    Dégâts : 5 - 12 | Critique : 20" << std::endl;
    std::cout << "    Potions de soin : 2" << std::endl;
    std::cout << "    Potions de dégâts : 5" << std::endl;
    std::cout << std::endl;
}

ClasseJoueur CatalogueClasses::creerClasseDeBase(int choix)
{
    if (choix == 1)
    {
        return ClasseJoueur("Chevalier", 200, 5, 20, 30, 4, 3);
    }

    if (choix == 2)
    {
        return ClasseJoueur("Classe Légère", 150, 10, 30, 45, 5, 2);
    }

    return ClasseJoueur("Classe Lourde", 400, 5, 12, 20, 2, 5);
}

ClasseJoueur CatalogueClasses::creerClasseEvolueeDepuisClasse(const std::string& classeActuelle)
{
    if (classeActuelle == "Chevalier")
    {
        return ClasseJoueur("Paladin", 500, 5, 20, 30, 6, 5);
    }

    if (classeActuelle == "Classe Légère")
    {
        return ClasseJoueur("Assassin", 400, 10, 30, 45, 7, 4);
    }

    return ClasseJoueur("Colosse", 900, 5, 12, 20, 4, 7);
}