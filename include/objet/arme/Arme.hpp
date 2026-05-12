#ifndef ARME_HPP
#define ARME_HPP

#include "objet/Objet.hpp"
#include "objet/arme/TypeArme.hpp"

class Arme : public Objet
{
private:
    TypeArme type;

    int bonusDegatsMin;
    int bonusDegatsMax;
    int bonusCritique;

    int durabilite;
    int durabiliteMax;

public:
    Arme();

    Arme(
        const std::string& nom,
        const std::string& description,
        int valeur,
        TypeArme type,
        int bonusDegatsMin,
        int bonusDegatsMax,
        int bonusCritique,
        int durabiliteMax
    );

    TypeArme getType() const;

    int getBonusDegatsMin() const;
    int getBonusDegatsMax() const;
    int getBonusCritique() const;

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