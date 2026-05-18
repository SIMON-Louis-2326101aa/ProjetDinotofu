#ifndef TOUR_IA_COMBAT_HPP
#define TOUR_IA_COMBAT_HPP

#include "core/Random.hpp"

#include "entite/Entite.hpp"

class TourIACombat
{
public:
    static bool jouer(
        Entite& ia,
        Entite& defenseur,
        Random& random,
        int soinPotion,
        int bonusPotionDegats
    );
};

#endif