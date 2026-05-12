#ifndef INVENTAIRE_HPP
#define INVENTAIRE_HPP

#include <vector>

#include "objet/arme/Arme.hpp"
#include "objet/armure/Armure.hpp"
#include "objet/consommable/Consommable.hpp"
#include "objet/consommable/TypeConsommable.hpp"

class Inventaire
{
private:
    std::vector<Arme> armes;
    std::vector<Armure> armures;
    std::vector<Consommable> consommables;

    int or_;

public:
    Inventaire();

    int getOr() const;
    void gagnerOr(int quantite);
    bool depenserOr(int quantite);

    int getNombreArmes() const;
    int getNombreArmures() const;
    int getNombreConsommables() const;
    int compterConsommables(TypeConsommable type) const;

    const std::vector<Arme>& getArmes() const;
    const std::vector<Armure>& getArmures() const;
    const std::vector<Consommable>& getConsommables() const;

    void ajouterArme(const Arme& arme);
    void ajouterArmure(const Armure& armure);
    void ajouterConsommable(const Consommable& consommable);

    bool possedeArme(int index) const;
    bool possedeArmure(int index) const;
    bool possedeConsommable(int index) const;

    Arme getArme(int index) const;
    Arme* getArmeModifiable(int index);

    Armure getArmure(int index) const;
    Armure* getArmureModifiable(int index);

    Consommable getConsommable(int index) const;

    int trouverPremierConsommable(TypeConsommable type) const;
    bool utiliserPremierConsommable(TypeConsommable type, Consommable& consommableUtilise);

    bool retirerConsommable(int index);

    void afficherArmes() const;
    void afficherArmures() const;
    void afficherConsommables() const;
    void afficher() const;
};

#endif