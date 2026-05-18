#ifndef MENU_EQUIPEMENT_HPP
#define MENU_EQUIPEMENT_HPP

#include "entite/Joueur.hpp"

class MenuEquipement
{
public:
    static bool ouvrir(Joueur& joueur);

    static bool equiperArmeDepuisInventaire(Joueur& joueur);
    static bool equiperArmureDepuisInventaire(Joueur& joueur);
};

#endif