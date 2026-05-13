#ifndef COMBAT_HPP
#define COMBAT_HPP

#include "entite/Joueur.hpp"
#include "core/Random.hpp"

class Combat
{
private:
    Random random;

public:
    void lancerPvpDeuxJoueurs(Joueur& joueur1);
    void lancerPvpIA(Joueur& joueur1);
    void lancerPveBoss(Joueur& joueur1);
};

#endif