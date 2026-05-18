#ifndef TOUR_BOSS_COMBAT_HPP
#define TOUR_BOSS_COMBAT_HPP

#include "core/Random.hpp"

#include "entite/Entite.hpp"
#include "entite/Boss.hpp"

class TourBossCombat
{
public:
    static bool jouer(
        Boss& boss,
        Entite& joueur,
        Random& random
    );
};

#endif