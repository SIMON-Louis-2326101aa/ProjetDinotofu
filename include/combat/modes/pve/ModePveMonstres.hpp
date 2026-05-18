#ifndef MODE_PVE_MONSTRES_HPP
#define MODE_PVE_MONSTRES_HPP

#include "core/Random.hpp"
#include "entite/Joueur.hpp"

class ModePveMonstres
{
public:
    static void lancer(Joueur& joueur, Random& random);
};

#endif