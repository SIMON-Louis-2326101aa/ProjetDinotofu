#ifndef CONSOMMABLE_HPP
#define CONSOMMABLE_HPP

#include "objet/Objet.hpp"
#include "objet/consommable/TypeConsommable.hpp"

class Consommable : public Objet
{
private:
    TypeConsommable type;
    int puissance;

public:
    Consommable();

    Consommable(
        const std::string& nom,
        const std::string& description,
        int valeur,
        TypeConsommable type,
        int puissance
    );

    TypeConsommable getType() const;
    int getPuissance() const;

    bool estSoin() const;
    bool estDegats() const;

    void afficher() const override;
};

#endif