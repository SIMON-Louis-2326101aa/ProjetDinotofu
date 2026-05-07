#ifndef CLASSE_JOUEUR_HPP
#define CLASSE_JOUEUR_HPP

#include <string>

class ClasseJoueur
{
private:
    std::string nom;
    int pvMax;
    int degatsMin;
    int degatsMax;
    int degatsCrit;
    int potionsSoin;
    int potionsDegats;

public:
    ClasseJoueur();

    ClasseJoueur(
        const std::string& nom,
        int pvMax,
        int degatsMin,
        int degatsMax,
        int degatsCrit,
        int potionsSoin,
        int potionsDegats
    );

    std::string getNom() const;
    int getPvMax() const;
    int getDegatsMin() const;
    int getDegatsMax() const;
    int getDegatsCrit() const;
    int getPotionsSoin() const;
    int getPotionsDegats() const;
};

#endif