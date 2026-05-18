#ifndef TOUR_HUMAIN_COMBAT_HPP
#define TOUR_HUMAIN_COMBAT_HPP

#include "core/Random.hpp"

#include "entite/Entite.hpp"

class TourHumainCombat
{
public:
    static bool jouer(
        Entite& attaquant,
        Entite& defenseur,
        Random& random,
        int soinPotion,
        int bonusPotionDegats
    );

private:
    static bool ouvrirInterfaceObservation(
        Entite& joueurInterface,
        Entite& cible
    );

    static bool gererFuite(
        Entite& attaquant,
        Entite& defenseur,
        Random& random
    );
};

#endif