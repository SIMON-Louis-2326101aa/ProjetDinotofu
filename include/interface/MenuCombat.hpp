#ifndef MENU_COMBAT_HPP
#define MENU_COMBAT_HPP

#include "entite/Entite.hpp"

class MenuCombat
{
public:
    static void afficherMenuTour(const Entite& entite);
    static void afficherManuelPotions(int soinPotion, int bonusPotionDegats);
    static void afficherPvApresAttaque(const Entite& defenseur);
    static void afficherResultatCombat(const Entite& joueur1, const Entite& joueur2);
};

#endif