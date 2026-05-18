#ifndef INVENTAIRE_SELECTION_HPP
#define INVENTAIRE_SELECTION_HPP

#include "entite/Joueur.hpp"

class InventaireSelection
{
public:
    static bool ouvrirArmes(Joueur& joueur);
    static bool ouvrirArmures(Joueur& joueur);
    static bool ouvrirConsommables(Joueur& joueur);
    static bool ouvrirMateriaux(Joueur& joueur);
};

#endif