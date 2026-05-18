#ifndef EQUIPEMENT_COMPARAISON_HPP
#define EQUIPEMENT_COMPARAISON_HPP

#include "entite/Joueur.hpp"

#include "objet/arme/Arme.hpp"
#include "objet/armure/Armure.hpp"

class EquipementComparaison
{
public:
    static void afficherComparaisonArme(const Joueur& joueur, const Arme& nouvelleArme);
    static void afficherComparaisonArmure(const Joueur& joueur, const Armure& nouvelleArmure);
};

#endif