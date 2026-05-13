#ifndef MODE_PVP_IA_HPP
#define MODE_PVP_IA_HPP

#include "entite/Joueur.hpp"
#include "core/Random.hpp"

class ModePvpIA
{
private:
    static constexpr int SOIN_POTION = 55;
    static constexpr int BONUS_POTION_DEGATS = 25;

public:
    static void lancer(Joueur& joueur1, Random& random);
};

#endif