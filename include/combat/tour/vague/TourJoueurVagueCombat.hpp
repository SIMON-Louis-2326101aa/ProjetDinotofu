#ifndef TOUR_JOUEUR_VAGUE_COMBAT_HPP
#define TOUR_JOUEUR_VAGUE_COMBAT_HPP

#include "core/Random.hpp"

#include "entite/Joueur.hpp"
#include "combat/FileEnnemisCombat.hpp"

class TourJoueurVagueCombat
{
public:
    static bool jouer(
        Joueur& joueur,
        FileEnnemisCombat& vague,
        Random& random,
        bool& fuiteReussie
    );

private:
    static constexpr int BONUS_POTION_DEGATS_PVE = 25;

    static bool ouvrirInterfaceVague(
        Joueur& joueur,
        FileEnnemisCombat& vague
    );
};

#endif