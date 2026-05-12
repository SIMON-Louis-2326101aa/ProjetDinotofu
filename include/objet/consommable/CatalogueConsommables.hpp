#ifndef CATALOGUE_CONSOMMABLES_HPP
#define CATALOGUE_CONSOMMABLES_HPP

#include "objet/consommable/Consommable.hpp"

class CatalogueConsommables
{
public:
    static Consommable creerPotionSoinBasique();
    static Consommable creerPotionDegatsBasique();

    static Consommable creerPotionSoinRenforcee();
    static Consommable creerPotionDegatsRenforcee();
};

#endif