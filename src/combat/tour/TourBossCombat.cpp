#include "combat/tour/TourBossCombat.hpp"

#include "combat/ActionsCombat.hpp"
#include "combat/BossCombat.hpp"
#include "combat/ia/IACombat.hpp"
#include "combat/ia/ActionIA.hpp"

#include "core/Console.hpp"

#include <iostream>

bool TourBossCombat::jouer(
    Boss& boss,
    Entite& joueur,
    Random& random
)
{
    std::cout << "Tour de " << boss.getNom() << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(1);

    ActionIA action = IACombat::choisirActionBoss(boss, random);

    if (action == ActionIA::Attaquer)
    {
        ActionsCombat::executerAttaque(boss, joueur, random);
        return BossCombat::gererFinTourBoss(boss, joueur);
    }

    if (action == ActionIA::PotionSoin)
    {
        bool potionUtilisee = boss.utiliserPotionSoin(boss.getPvMax() * 10 / 100);

        if (potionUtilisee)
        {
            std::cout << boss.getNom() << " récupère une partie de sa vitalité." << std::endl;
            std::cout << std::endl;
        }
        else
        {
            std::cout << boss.getNom()
                      << " cherche une source de régénération, mais rien ne répond."
                      << std::endl;
            std::cout << std::endl;

            ActionsCombat::executerAttaque(boss, joueur, random);
        }

        return BossCombat::gererFinTourBoss(boss, joueur);
    }

    if (action == ActionIA::PotionDegats)
    {
        bool potionUtilisee = ActionsCombat::executerPotionDegats(
            boss,
            joueur,
            random,
            50
        );

        if (!potionUtilisee)
        {
            ActionsCombat::executerAttaque(boss, joueur, random);
        }

        return BossCombat::gererFinTourBoss(boss, joueur);
    }

    if (action == ActionIA::Ultime)
    {
        BossCombat::executerUltimeBoss(boss, joueur, random);
        return BossCombat::gererFinTourBoss(boss, joueur);
    }

    std::cout << boss.getNom()
              << " reste immobile, comme s'il observait déjà ta fin."
              << std::endl;
    std::cout << std::endl;

    return BossCombat::gererFinTourBoss(boss, joueur);
}