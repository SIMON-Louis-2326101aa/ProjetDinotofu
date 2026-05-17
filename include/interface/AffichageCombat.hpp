#ifndef AFFICHAGE_COMBAT_HPP
#define AFFICHAGE_COMBAT_HPP

#include "entite/Entite.hpp"

class AffichageCombat
{
public:
    static void afficherResultatCombat(
        const Entite& combattant1,
        const Entite& combattant2
    );
};

#endif