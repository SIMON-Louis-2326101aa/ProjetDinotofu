#include "combat/action/EffetsSpeciauxCombat.hpp"

#include "core/Console.hpp"

#include "entite/Boss.hpp"

#include <iostream>

bool EffetsSpeciauxCombat::atlasBloqueAttaque(
    Entite& attaquant,
    Entite& defenseur,
    int degats
)
{
    Boss* bossDefenseur = dynamic_cast<Boss*>(&defenseur);

    if (bossDefenseur == nullptr)
    {
        return false;
    }

    if (bossDefenseur->getIdBoss() != 3 || !bossDefenseur->ultimeActif())
    {
        return false;
    }

    int degatsRenvoi = degats / 3;

    attaquant.recevoirDegats(degatsRenvoi);

    std::cout << attaquant.getNom()
              << " frappe de toutes ses forces..."
              << std::endl;

    Console::pauseSecondes(1);

    std::cout << "Mais l'armure d'"
              << bossDefenseur->getNom()
              << " absorbe l'impact."
              << std::endl;

    std::cout << "Une partie de la puissance est renvoyée à "
              << attaquant.getNom()
              << ", qui subit "
              << degatsRenvoi
              << " dégâts."
              << std::endl;

    std::cout << std::endl;

    std::cout << attaquant.getNom()
              << " possède maintenant "
              << attaquant.getPv()
              << "/"
              << attaquant.getPvMax()
              << " PV."
              << std::endl;

    std::cout << std::endl;

    return true;
}

void EffetsSpeciauxCombat::appliquerVolDeVieDemonSiBesoin(
    Entite& attaquant,
    int degatsInfliges
)
{
    Boss* bossAttaquant = dynamic_cast<Boss*>(&attaquant);

    if (bossAttaquant == nullptr)
    {
        return;
    }

    if (bossAttaquant->getIdBoss() != 2 || !bossAttaquant->ultimeActif())
    {
        return;
    }

    if (bossAttaquant->getEffetSpecial() != 2
        && bossAttaquant->getEffetSpecial() != 3)
    {
        return;
    }

    int soin = degatsInfliges * 50 / 100;

    if (soin <= 0)
    {
        return;
    }

    bossAttaquant->soigner(soin);

    std::cout << bossAttaquant->getNom()
              << " absorbe le sang de l'attaque et récupère "
              << soin
              << " PV."
              << std::endl;

    std::cout << std::endl;
}