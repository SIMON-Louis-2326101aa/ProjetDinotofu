#ifndef BOSS_COMBAT_HPP
#define BOSS_COMBAT_HPP

#include "core/Random.hpp"
#include "entite/Boss.hpp"
#include "entite/Entite.hpp"

class BossCombat
{
public:
    static void verifierDecryptageBoss(Boss& boss);

    static void executerUltimeBoss(
        Boss& boss,
        Entite& joueur,
        Random& random
    );

    static bool gererFinTourBoss(
        Boss& boss,
        Entite& joueur
    );
};

#endif