#ifndef ARMURE_HPP
#define ARMURE_HPP

#include "objet/Objet.hpp"
#include "objet/armure/TypeArmure.hpp"

class Armure : public Objet
{
private:
    TypeArmure type;

    int bonusPvMax;
    int reductionDegats;

    int durabilite;
    int durabiliteMax;

public:
    Armure();

    Armure(
        const std::string& nom,
        const std::string& description,
        int valeur,
        TypeArmure type,
        int bonusPvMax,
        int reductionDegats,
        int durabiliteMax
    );

    TypeArmure getType() const;

    int getBonusPvMax() const;
    int getReductionDegats() const;

    int getDurabilite() const;
    int getDurabiliteMax() const;

    bool estCassee() const;
    bool estIndestructible() const;

    void perdreDurabilite(int quantite);
    void reparer(int quantite);
    void reparerCompletement();

    void afficher() const override;
};

#endif