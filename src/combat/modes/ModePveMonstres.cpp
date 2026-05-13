#include "combat/modes/ModePveMonstres.hpp"

#include "core/Console.hpp"

#include <iostream>

void ModePveMonstres::lancer(Joueur& joueur, Random& random)
{
    (void)joueur;
    (void)random;

    std::cout << "Le mode PvE contre monstres n'est pas encore disponible." << std::endl;
    std::cout << "Mais l'arène prépare déjà ses premières vagues..." << std::endl;
    std::cout << std::endl;

    Console::attendreEntree();
}