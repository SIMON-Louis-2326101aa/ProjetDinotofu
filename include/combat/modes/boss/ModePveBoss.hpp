#ifndef MODE_PVE_BOSS_HPP
#define MODE_PVE_BOSS_HPP

#include "entite/Joueur.hpp"
#include "core/Random.hpp"

class ModePveBoss
{
private:
    static constexpr int SOIN_POTION_BOSS = 75;
    static constexpr int BONUS_POTION_DEGATS_BOSS = 45;

public:
    static void lancer(Joueur& joueur1, Random& random);
};

#endif