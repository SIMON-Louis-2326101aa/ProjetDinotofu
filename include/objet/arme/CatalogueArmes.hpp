#ifndef CATALOGUE_ARMES_HPP
#define CATALOGUE_ARMES_HPP

#include "objet/arme/Arme.hpp"

class CatalogueArmes
{
public:
    static Arme creerMainsNues();
    static Arme creerEpeeRouillee();
    static Arme creerLameArene();
};

#endif