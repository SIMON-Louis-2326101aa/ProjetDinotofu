#include "interface/AffichageCombat.hpp"

#include <iostream>

void AffichageCombat::afficherResultatCombat(
    const Entite& combattant1,
    const Entite& combattant2
)
{
    std::cout << std::endl;
    std::cout << "========== RÉSULTAT DU COMBAT ==========" << std::endl;

    if (combattant1.estMort() && combattant2.estMort())
    {
        std::cout << "Les deux combattants s'effondrent en même temps." << std::endl;
        std::cout << "L'arène reste silencieuse... aucun vainqueur clair." << std::endl;
    }
    else if (combattant1.estMort())
    {
        std::cout << combattant1.getNom() << " tombe au sol." << std::endl;
        std::cout << combattant2.getNom() << " remporte le duel." << std::endl;
    }
    else if (combattant2.estMort())
    {
        std::cout << combattant2.getNom() << " tombe au sol." << std::endl;
        std::cout << combattant1.getNom() << " remporte le duel." << std::endl;
    }
    else
    {
        std::cout << "Le combat s'arrête sans vainqueur." << std::endl;
    }

    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
}