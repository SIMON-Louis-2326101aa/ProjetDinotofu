#ifndef CATALOGUE_MONSTRES_HPP
#define CATALOGUE_MONSTRES_HPP

#include "entite/Monstre.hpp"

class CatalogueMonstres
{
public:
    static Monstre creerGobelin();
    static Monstre creerGobelinBrutal();
    static Monstre creerLoupAffame();
    static Monstre creerSquelette();
    static Monstre creerOrcMineur();

    static void afficherMonstresDisponibles();
};

#endif