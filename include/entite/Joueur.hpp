#ifndef JOUEUR_HPP
#define JOUEUR_HPP

#include "entite/Entite.hpp"
#include "classe/ClasseJoueur.hpp"

class Joueur : public Entite
{
private:
    int niveau;
    int experience;
    int argent;

public:
    Joueur();

    Joueur(
        const std::string& nom,
        const ClasseJoueur& classeChoisie
    );

    int getNiveau() const;
    int getExperience() const;
    int getArgent() const;

    void afficherStats() const override;
};

#endif
