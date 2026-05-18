#ifndef EFFETS_SPECIAUX_COMBAT_HPP
#define EFFETS_SPECIAUX_COMBAT_HPP

#include "entite/Entite.hpp"

class EffetsSpeciauxCombat
{
public:
    static bool atlasBloqueAttaque(
        Entite& attaquant,
        Entite& defenseur,
        int degats
    );

    static void appliquerVolDeVieDemonSiBesoin(
        Entite& attaquant,
        int degatsInfliges
    );
};

#endif