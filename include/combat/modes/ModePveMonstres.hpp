#ifndef MODE_PVE_MONSTRES_HPP
#define MODE_PVE_MONSTRES_HPP

#include "entite/Joueur.hpp"
#include "entite/Monstre.hpp"
#include "combat/FileEnnemisCombat.hpp"
#include "core/Random.hpp"

class ModePveMonstres
{
private:
    static constexpr int SOIN_POTION_PVE = 55;
    static constexpr int BONUS_POTION_DEGATS_PVE = 25;

    static bool jouerTourJoueur(
        Joueur& joueur,
        FileEnnemisCombat& vague,
        Random& random,
        bool& fuiteReussie
    );

    static void jouerToursMonstres(
        Joueur& joueur,
        FileEnnemisCombat& vague,
        Random& random
    );

public:
    static void lancer(Joueur& joueur, Random& random);
};

#endif