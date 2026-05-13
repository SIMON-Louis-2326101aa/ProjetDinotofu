#ifndef MENU_COMBAT_HPP
#define MENU_COMBAT_HPP

#include "entite/Entite.hpp"

class MenuCombat
{
public:
    static void afficherMenuTour(const Entite& entite);
    static void afficherOptionNonDisponible();
};

#endif