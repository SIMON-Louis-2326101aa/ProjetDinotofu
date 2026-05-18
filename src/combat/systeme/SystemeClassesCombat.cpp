#include "combat/systeme/SystemeClassesCombat.hpp"

#include <algorithm>
#include <cctype>
#include <string>

std::string SystemeClassesCombat::normaliserTexteClasse(const std::string& texteClasse)
{
    std::string resultat = texteClasse;

    std::transform(
        resultat.begin(),
        resultat.end(),
        resultat.begin(),
        [](unsigned char caractere)
        {
            return static_cast<char>(std::tolower(caractere));
        }
    );

    return resultat;
}

int SystemeClassesCombat::getChanceFuiteBase(const Entite& entite)
{
    std::string classe = normaliserTexteClasse(entite.getType());

    if (classe.find("assassin") != std::string::npos ||
        classe.find("légère") != std::string::npos ||
        classe.find("legere") != std::string::npos)
    {
        return 70;
    }

    if (classe.find("colosse") != std::string::npos ||
        classe.find("lourde") != std::string::npos)
    {
        return 30;
    }

    if (classe.find("chevalier") != std::string::npos ||
        classe.find("paladin") != std::string::npos)
    {
        return 50;
    }

    return 50;
}

int SystemeClassesCombat::getReductionDegatsBasePourcentage(const Entite& entite)
{
    std::string classe = normaliserTexteClasse(entite.getType());

    if (classe.find("assassin") != std::string::npos ||
        classe.find("légère") != std::string::npos ||
        classe.find("legere") != std::string::npos)
    {
        return 0;
    }

    if (classe.find("colosse") != std::string::npos ||
        classe.find("lourde") != std::string::npos)
    {
        return 10;
    }

    if (classe.find("chevalier") != std::string::npos ||
        classe.find("paladin") != std::string::npos)
    {
        return 5;
    }

    return 0;
}