// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "class_system/ClassCatalog.hpp"

#include <iostream>

void ClassCatalog::displayBasicClasses()
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

PlayerClass ClassCatalog::createBaseClass(int choice)
{
    if (choice == 1)
    {
        return PlayerClass("Chevalier", 200, 5, 20, 30, 4, 3);
    }

    if (choice == 2)
    {
        return PlayerClass("Classe Légère", 150, 10, 30, 45, 5, 2);
    }

    return PlayerClass("Classe Lourde", 400, 5, 12, 20, 2, 5);
}

PlayerClass ClassCatalog::createEvolvedClassFromClass(const std::string& currentClass)
{
    if (currentClass == "Chevalier")
    {
        return PlayerClass("Paladin", 500, 5, 20, 30, 6, 5);
    }

    if (currentClass == "Classe Légère")
    {
        return PlayerClass("Assassin", 400, 10, 30, 45, 7, 4);
    }

    return PlayerClass("Colosse", 900, 5, 12, 20, 4, 7);
}