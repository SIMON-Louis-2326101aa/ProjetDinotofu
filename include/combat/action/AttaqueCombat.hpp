#ifndef ATTAQUE_COMBAT_HPP
#define ATTAQUE_COMBAT_HPP

#include "core/Random.hpp"
#include "entite/Entite.hpp"

class AttaqueCombat
{
public:
    static void executerAttaque(
        Entite& attaquant,
        Entite& defenseur,
        Random& random
    );

    static void executerAttaqueBoostee(
        Entite& attaquant,
        Entite& defenseur,
        Random& random,
        int bonusDegats
    );
};

#endif