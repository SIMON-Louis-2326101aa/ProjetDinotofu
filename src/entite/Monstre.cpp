#include "entite/Monstre.hpp"

#include <iostream>

Monstre::Monstre()
    : Entite(),
      niveau(1),
      race(Race::Inconnue),
      invocation(false),
      elite(false),
      statsCachees(false)
{
}

Monstre::Monstre(
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
    bool invocation,
    bool elite,
    bool statsCachees
)
    : Entite(
          nom,
          type,
          pvMax,
          degatsMin,
          degatsMax,
          degatsCrit,
          potionsSoin,
          potionsDegats
      ),
      niveau(niveau),
      race(race),
      invocation(invocation),
      elite(elite),
      statsCachees(statsCachees)
{
}

int Monstre::getNiveau() const
{
    return niveau;
}

Race Monstre::getRace() const
{
    return race;
}

std::string Monstre::getRaceTexte() const
{
    return raceVersTexte(race);
}

bool Monstre::estInvocation() const
{
    return invocation;
}

bool Monstre::estElite() const
{
    return elite;
}

bool Monstre::statsVisibles() const
{
    return !statsCachees;
}

void Monstre::revelerStats()
{
    statsCachees = false;
}

void Monstre::afficherStats() const
{
    if (!statsVisibles())
    {
        std::cout << "========== DONNÉES BROUILLÉES ==========" << std::endl;
        std::cout << "Nom : " << nom << std::endl;
        std::cout << "Race : " << getRaceTexte() << std::endl;
        std::cout << "Type : " << type << std::endl;
        std::cout << "Niveau : ???" << std::endl;
        std::cout << "PV : ???" << std::endl;
        std::cout << "Dégâts : ???" << std::endl;
        std::cout << "Critique : ???" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << std::endl;
        return;
    }

    std::cout << "========== STATS MONSTRE ==========" << std::endl;
    std::cout << "Nom : " << nom << std::endl;
    std::cout << "Race : " << getRaceTexte() << std::endl;
    std::cout << "Type : " << type << std::endl;
    std::cout << "Niveau : " << niveau << std::endl;
    std::cout << "PV : " << pv << "/" << pvMax << std::endl;
    std::cout << "Dégâts : " << degatsMin << " - " << degatsMax << std::endl;
    std::cout << "Critique : " << degatsCrit << std::endl;

    if (invocation)
    {
        std::cout << "Statut : Invocation" << std::endl;
    }
    else if (elite)
    {
        std::cout << "Statut : Élite" << std::endl;
    }
    else
    {
        std::cout << "Statut : Monstre standard" << std::endl;
    }

    std::cout << "===================================" << std::endl;
    std::cout << std::endl;
}