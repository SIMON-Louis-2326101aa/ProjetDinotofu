#ifndef MODE_PVE_MONSTRES_HPP
#define MODE_PVE_MONSTRES_HPP

#include "entite/Joueur.hpp"
#include "core/Random.hpp"

class ModePveMonstres
{
public:
    static void lancer(Joueur& joueur, Random& random);
};

#endif