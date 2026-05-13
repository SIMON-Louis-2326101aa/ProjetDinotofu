#ifndef MENU_INVENTAIRE_HPP
#define MENU_INVENTAIRE_HPP

#include "entite/Joueur.hpp"

class MenuInventaire
{
private:
    static bool ouvrirArmes(Joueur& joueur);
    static bool ouvrirArmures(Joueur& joueur);
    static bool ouvrirConsommables(Joueur& joueur);

public:
    static bool ouvrir(Joueur& joueur);
};

#endif