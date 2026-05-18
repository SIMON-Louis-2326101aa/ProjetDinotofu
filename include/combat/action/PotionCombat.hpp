#ifndef POTION_COMBAT_HPP
#define POTION_COMBAT_HPP

#include "core/Random.hpp"
#include "entite/Entite.hpp"

class PotionCombat
{
public:
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
};

#endif