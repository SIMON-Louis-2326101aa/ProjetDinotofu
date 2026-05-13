#ifndef GESTIONNAIRE_TOURS_HPP
#define GESTIONNAIRE_TOURS_HPP

#include "entite/Entite.hpp"
#include "entite/Joueur.hpp"
#include "entite/Boss.hpp"
#include "core/Random.hpp"

class GestionnaireTours
{
public:
    static bool jouerTourHumain(
        Entite& attaquant,
        Entite& defenseur,
        Random& random,
        int soinPotion,
        int bonusPotionDegats
    );

    static bool jouerTourIA(
        Entite& ia,
        Entite& defenseur,
        Random& random,
        int soinPotion,
        int bonusPotionDegats
    );

    static bool jouerTourBoss(
        Boss& boss,
        Entite& joueur,
        Random& random
    );

private:
    static void executerAttaque(
        Entite& attaquant,
        Entite& defenseur,
        Random& random
    );

    static bool executerPotionSoin(
        Entite& entite,
        int soinPotion
    );

    static bool executerPotionDegats(
        Entite& attaquant,
        Entite& defenseur,
        Random& random,
        int bonusPotionDegats
    );

    static bool atlasBloqueAttaque(
        Entite& attaquant,
        Entite& defenseur,
        int degats
    );

    static void appliquerVolDeVieDemonSiBesoin(
        Entite& attaquant,
        int degatsInfliges
    );

    static void verifierDecryptageBoss(Boss& boss);

    static void executerUltimeBoss(
        Boss& boss,
        Entite& joueur,
        Random& random
    );

    static bool gererFinTourBoss(
        Boss& boss,
        Entite& joueur
    );
};

#endif