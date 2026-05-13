#ifndef MONSTRE_HPP
#define MONSTRE_HPP

#include "entite/Entite.hpp"
#include "entite/Race.hpp"

class Monstre : public Entite
{
private:
    int niveau;
    Race race;

    bool invocation;
    bool elite;
    bool statsCachees;

public:
    Monstre();

    Monstre(
        const std::string& nom,
        const std::string& type,
        Race race,
        int niveau,
        int pvMax,
        int degatsMin,
        int degatsMax,
        int degatsCrit,
        int potionsSoin,
        int potionsDegats,
        bool invocation = false,
        bool elite = false,
        bool statsCachees = false
    );

    int getNiveau() const;
    Race getRace() const;
    std::string getRaceTexte() const;

    bool estInvocation() const;
    bool estElite() const;

    bool statsVisibles() const override;
    void revelerStats();

    void afficherStats() const override;
};

#endif