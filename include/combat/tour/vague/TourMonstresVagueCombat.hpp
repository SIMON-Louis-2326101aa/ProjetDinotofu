#ifndef TOUR_MONSTRES_VAGUE_COMBAT_HPP
#define TOUR_MONSTRES_VAGUE_COMBAT_HPP

#include "core/Random.hpp"

#include "entite/Joueur.hpp"
#include "combat/FileEnnemisCombat.hpp"

class TourMonstresVagueCombat
{
public:
    static void jouerToursMonstres(
        Joueur& joueur,
        FileEnnemisCombat& vague,
        Random& random
    );
};

#endif