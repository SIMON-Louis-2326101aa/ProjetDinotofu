#ifndef BOSS_ULTIME_HPP
#define BOSS_ULTIME_HPP

#include "core/Random.hpp"

#include "entite/Boss.hpp"
#include "entite/Entite.hpp"

class BossUltime
{
public:
    static void executerUltimeBoss(
        Boss& boss,
        Entite& joueur,
        Random& random
    );
};

#endif