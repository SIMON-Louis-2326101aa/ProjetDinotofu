#ifndef CATALOGUE_CLASSES_HPP
#define CATALOGUE_CLASSES_HPP

#include "classe/ClasseJoueur.hpp"

class CatalogueClasses
{
public:
    static void afficherClassesDeBase();

    static ClasseJoueur creerClasseDeBase(int choix);
    static ClasseJoueur creerClasseEvolueeDepuisClasse(const std::string& classeActuelle);
};

#endif