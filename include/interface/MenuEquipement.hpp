#ifndef MENU_EQUIPEMENT_HPP
#define MENU_EQUIPEMENT_HPP

#include "entite/Joueur.hpp"

class MenuEquipement
{
private:
    static void afficherComparaisonArme(const Joueur& joueur, const Arme& nouvelleArme);
    static void afficherComparaisonArmure(const Joueur& joueur, const Armure& nouvelleArmure);

public:
    static bool ouvrir(Joueur& joueur);

    static bool equiperArmeDepuisInventaire(Joueur& joueur);
    static bool equiperArmureDepuisInventaire(Joueur& joueur);
};

#endif