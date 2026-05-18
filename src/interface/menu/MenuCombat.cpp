#include "interface/menu/MenuCombat.hpp"

#include <iostream>

void MenuCombat::afficherMenuTour(const Entite& entite)
{
    std::cout << "========== MENU COMBAT ==========" << std::endl;
    std::cout << "Tour de " << entite.getNom() << std::endl;
    std::cout << "---------------------------------" << std::endl;
    std::cout << "0 : Interface" << std::endl;
    std::cout << "1 : Attaquer" << std::endl;
    std::cout << "2 : Potion de soin rapide" << std::endl;
    std::cout << "3 : Potions" << std::endl;
    std::cout << "4 : Équipement" << std::endl;
    std::cout << "5 : Inventaire" << std::endl;
    std::cout << "6 : Passer son tour" << std::endl;
    std::cout << "7 : Fuir" << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void MenuCombat::afficherOptionNonDisponible()
{
    std::cout << "[cette option n'est pas encore accessible pour ce mode]" << std::endl;
    std::cout << std::endl;
}