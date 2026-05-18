#include "combat/tour/TourIACombat.hpp"

#include "combat/ActionsCombat.hpp"
#include "combat/ia/IACombat.hpp"
#include "combat/ia/ActionIA.hpp"

#include "core/Console.hpp"

#include <iostream>

bool TourIACombat::jouer(
    Entite& ia,
    Entite& defenseur,
    Random& random,
    int soinPotion,
    int bonusPotionDegats
)
{
    std::cout << "Tour de " << ia.getNom() << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(1);

    ActionIA action = IACombat::choisirActionIA(ia, random);

    if (action == ActionIA::Attaquer)
    {
        ActionsCombat::executerAttaque(ia, defenseur, random);
        return true;
    }

    if (action == ActionIA::PotionSoin)
    {
        bool actionReussie = ActionsCombat::executerPotionSoin(ia, soinPotion);

        if (!actionReussie)
        {
            ActionsCombat::executerAttaque(ia, defenseur, random);
        }

        return true;
    }

    if (action == ActionIA::PotionDegats)
    {
        bool actionReussie = ActionsCombat::executerPotionDegats(
            ia,
            defenseur,
            random,
            bonusPotionDegats
        );

        if (!actionReussie)
        {
            ActionsCombat::executerAttaque(ia, defenseur, random);
        }

        return true;
    }

    std::cout << ia.getNom() << " hésite, fixe le vide, et passe son tour." << std::endl;
    std::cout << std::endl;

    return true;
}