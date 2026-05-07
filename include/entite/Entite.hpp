#ifndef ENTITE_HPP
#define ENTITE_HPP

#include <string>

#include "core/Random.hpp"
#include "classe/ClasseJoueur.hpp"

class Entite
{
protected:
    std::string nom;
    std::string type;

    int pv;
    int pvMax;

    int degatsMin;
    int degatsMax;
    int degatsCrit;

    int potionsSoin;
    int potionsDegats;

public:
    Entite();

    Entite(
        const std::string& nom,
        const std::string& type,
        int pvMax,
        int degatsMin,
        int degatsMax,
        int degatsCrit,
        int potionsSoin,
        int potionsDegats
    );

    virtual ~Entite() = default;

    std::string getNom() const;
    std::string getType() const;

    int getPv() const;
    int getPvMax() const;

    int getDegatsMin() const;
    int getDegatsMax() const;
    int getDegatsCrit() const;

    int getPotionsSoin() const;
    int getPotionsDegats() const;

    bool estMort() const;

    void recevoirDegats(int degats);
    void soigner(int valeurSoin);
    void reduirePvMax(int valeur);

    int attaquer(Random& random, bool& esquive, bool& critique, int bonusDegats = 0);

    bool utiliserPotionSoin(int valeurSoin);
    bool consommerPotionDegats();

    void appliquerClasse(const ClasseJoueur& nouvelleClasse);

    virtual bool statsVisibles() const;
    virtual void afficherStats() const;
};

#endif
