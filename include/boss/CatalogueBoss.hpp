#ifndef CATALOGUE_BOSS_HPP
#define CATALOGUE_BOSS_HPP

#include "entite/Boss.hpp"

class CatalogueBoss
{
public:
    static void afficherBossDisponibles();
    static Boss creerBoss(int choix);
};

#endif