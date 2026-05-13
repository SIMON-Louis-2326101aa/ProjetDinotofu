#ifndef ACTIONS_COMBAT_HPP
#define ACTIONS_COMBAT_HPP

#include "core/Random.hpp"
#include "entite/Entite.hpp"

class ActionsCombat
{
public:
    static void executerAttaque(
        Entite& attaquant,
        Entite& defenseur,
        Random& random
    );

    static bool executerPotionSoin(
        Entite& entite,
        int soinPotion
    );

    static bool executerPotionDegats(
        Entite& attaquant,
        Entite& defenseur,
        Random& random,
        int bonusPotionDegats
    );

private:
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