#ifndef INVENTAIRE_AFFICHAGE_HPP
#define INVENTAIRE_AFFICHAGE_HPP

#include "entite/Joueur.hpp"

#include "objet/arme/Arme.hpp"
#include "objet/armure/Armure.hpp"
#include "objet/consommable/Consommable.hpp"

class InventaireAffichage
{
public:
    static void afficherMenuPrincipal();

    static void afficherToutSimple(const Joueur& joueur);

    static void afficherArmeSelectionnee(const Arme& arme);
    static void afficherArmureSelectionnee(const Armure& armure);
    static void afficherConsommableSelectionne(const Consommable& consommable);

    static void afficherMateriauxIndisponibles();
};

#endif