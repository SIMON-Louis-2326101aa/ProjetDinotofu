#ifndef MODE_PVP_HPP
#define MODE_PVP_HPP

#include "entite/Joueur.hpp"
#include "core/Random.hpp"

class ModePvp
{
private:
    static constexpr int SOIN_POTION = 55;
    static constexpr int BONUS_POTION_DEGATS = 25;

public:
    static void lancer(Joueur& joueur1, Random& random);
};

#endif