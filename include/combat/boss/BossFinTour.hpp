#ifndef BOSS_FIN_TOUR_HPP
#define BOSS_FIN_TOUR_HPP

#include "entite/Boss.hpp"
#include "entite/Entite.hpp"

class BossFinTour
{
public:
    static bool gererFinTourBoss(
        Boss& boss,
        Entite& joueur
    );
};

#endif