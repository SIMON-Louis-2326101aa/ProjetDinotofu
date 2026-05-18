#include "combat/action/AttaqueCombat.hpp"

#include "combat/RapportDegats.hpp"
#include "combat/systeme/SystemeDegats.hpp"
#include "combat/action/EffetsSpeciauxCombat.hpp"

#include <iostream>

void AttaqueCombat::executerAttaque(
    Entite& attaquant,
    Entite& defenseur,
    Random& random
)
{
    executerAttaqueBoostee(
        attaquant,
        defenseur,
        random,
        0
    );
}

void AttaqueCombat::executerAttaqueBoostee(
    Entite& attaquant,
    Entite& defenseur,
    Random& random,
    int bonusDegats
)
{
    bool esquive = false;
    bool critique = false;

    int degatsBruts = attaquant.attaquer(
        random,
        esquive,
        critique,
        bonusDegats
    );

    if (esquive)
    {
        std::cout << attaquant.getNom()
                  << " attaque, mais "
                  << defenseur.getNom()
                  << " esquive au dernier moment."
                  << std::endl;
        std::cout << std::endl;
        return;
    }

    if (EffetsSpeciauxCombat::atlasBloqueAttaque(
        attaquant,
        defenseur,
        degatsBruts
    ))
    {
        return;
    }

    if (critique)
    {
        std::cout << attaquant.getNom()
                  << " frappe avec une violence monstrueuse et inflige "
                  << degatsBruts
                  << " dégâts bruts critiques."
                  << std::endl;
    }
    else if (bonusDegats > 0)
    {
        std::cout << attaquant.getNom()
                  << " attaque avec une puissance renforcée et inflige "
                  << degatsBruts
                  << " dégâts bruts."
                  << std::endl;
    }
    else
    {
        std::cout << attaquant.getNom()
                  << " attaque et inflige "
                  << degatsBruts
                  << " dégâts bruts."
                  << std::endl;
    }

    RapportDegats rapport = SystemeDegats::calculerDegatsRecus(
        defenseur,
        degatsBruts
    );

    SystemeDegats::afficherRapportDegats(
        defenseur,
        rapport
    );

    defenseur.recevoirDegats(rapport.degatsRecus);

    EffetsSpeciauxCombat::appliquerVolDeVieDemonSiBesoin(
        attaquant,
        rapport.degatsRecus
    );

    std::cout << defenseur.getNom()
              << " reçoit "
              << rapport.degatsRecus
              << " dégâts."
              << std::endl;

    std::cout << defenseur.getNom()
              << " possède maintenant "
              << defenseur.getPv()
              << "/"
              << defenseur.getPvMax()
              << " PV."
              << std::endl;

    std::cout << std::endl;
}