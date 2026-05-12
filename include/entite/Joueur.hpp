#ifndef JOUEUR_HPP
#define JOUEUR_HPP

#include "entite/Entite.hpp"
#include "classe/ClasseJoueur.hpp"
#include "objet/Inventaire.hpp"

class Joueur : public Entite
{
private:
    int niveau;
    int experience;

    Inventaire inventaire;
    int indexArmeEquipee;

public:
    Joueur();

    Joueur(
        const std::string& nom,
        const ClasseJoueur& classe
    );

    int getNiveau() const;
    int getExperience() const;

    Inventaire& getInventaire();
    const Inventaire& getInventaire() const;

    int getIndexArmeEquipee() const;
    bool aUneArmeEquipee() const;
    Arme getArmeEquipee() const;
    bool equiperArme(int index);
    void desequiperArme();

    void initialiserInventaireDeBase();

    void gagnerExperience(int quantite);
    void monterNiveau();

    int attaquer(Random& random, bool& esquive, bool& critique, int bonusDegats = 0) override;

    void afficherStats() const override;
    void afficherInventaire() const;
};

#endif
