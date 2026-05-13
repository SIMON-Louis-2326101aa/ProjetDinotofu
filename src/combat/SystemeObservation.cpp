#include "combat/SystemeObservation.hpp"

#include <iostream>

void SystemeObservation::afficherTentativeDecryptageEchouee(const Entite& cible)
{
    std::cout << "Tentative de décryptage échouée." << std::endl;
    std::cout << "Les données de " << cible.getNom()
              << " restent brouillées par une force beaucoup trop élevée." << std::endl;
    std::cout << "L'interface refuse encore d'afficher ses statistiques." << std::endl;
    std::cout << std::endl;
}

void SystemeObservation::afficherStatsTerminal(const Entite& cible)
{
    if (cible.statsVisibles())
    {
        cible.afficherStats();
        return;
    }

    afficherTentativeDecryptageEchouee(cible);
}