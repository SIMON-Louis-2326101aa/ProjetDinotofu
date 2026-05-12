#ifndef INVENTAIRE_HPP
#define INVENTAIRE_HPP

#include <vector>

#include "objet/arme/Arme.hpp"
#include "objet/consommable/Consommable.hpp"

class Inventaire
{
private:
    std::vector<Arme> armes;
    std::vector<Consommable> consommables;

    int or_;

public:
    Inventaire();

    int getOr() const;
    void gagnerOr(int quantite);
    bool depenserOr(int quantite);

    int getNombreArmes() const;
    int getNombreConsommables() const;

    const std::vector<Arme>& getArmes() const;
    const std::vector<Consommable>& getConsommables() const;

    void ajouterArme(const Arme& arme);
    void ajouterConsommable(const Consommable& consommable);

    bool possedeArme(int index) const;
    bool possedeConsommable(int index) const;

    Arme getArme(int index) const;
    Arme* getArmeModifiable(int index);

    Consommable getConsommable(int index) const;

    bool retirerConsommable(int index);

    void afficherArmes() const;
    void afficherConsommables() const;
    void afficher() const;
};

#endif