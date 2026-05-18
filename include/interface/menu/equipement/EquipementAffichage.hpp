#ifndef EQUIPEMENT_AFFICHAGE_HPP
#define EQUIPEMENT_AFFICHAGE_HPP

#include "entite/Joueur.hpp"

#include "objet/arme/Arme.hpp"
#include "objet/armure/Armure.hpp"

#include <string>

class EquipementAffichage
{
public:
    static void afficherMenuPrincipal();

    static void afficherListeArmes(const Joueur& joueur);
    static void afficherListeArmures(const Joueur& joueur);

    static void afficherArmeSelectionnee(const Arme& arme);
    static void afficherArmureSelectionnee(const Armure& armure);

    static void afficherResumeArme(const Arme& arme, int index);
    static void afficherResumeArmure(const Armure& armure, int index);

    static std::string durabiliteArmeTexte(const Arme& arme);
    static std::string durabiliteArmureTexte(const Armure& armure);
};

#endif