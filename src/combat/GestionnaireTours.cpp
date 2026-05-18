#include "combat/GestionnaireTours.hpp"

#include "combat/ActionsCombat.hpp"
#include "combat/BossCombat.hpp"

#include "combat/tour/TourHumainCombat.hpp"
#include "combat/tour/TourIACombat.hpp"
#include "combat/tour/TourBossCombat.hpp"

bool GestionnaireTours::jouerTourHumain(
    Entite& attaquant,
    Entite& defenseur,
    Random& random,
    int soinPotion,
    int bonusPotionDegats
)
{
    return TourHumainCombat::jouer(
        attaquant,
        defenseur,
        random,
        soinPotion,
        bonusPotionDegats
    );
}

bool GestionnaireTours::jouerTourIA(
    Entite& ia,
    Entite& defenseur,
    Random& random,
    int soinPotion,
    int bonusPotionDegats
)
{
    return TourIACombat::jouer(
        ia,
        defenseur,
        random,
        soinPotion,
        bonusPotionDegats
    );
}

bool GestionnaireTours::jouerTourBoss(
    Boss& boss,
    Entite& joueur,
    Random& random
)
{
    return TourBossCombat::jouer(
        boss,
        joueur,
        random
    );
}

void GestionnaireTours::verifierDecryptageBoss(Boss& boss)
{
    BossCombat::verifierDecryptageBoss(boss);
}

void GestionnaireTours::executerAttaque(
    Entite& attaquant,
    Entite& defenseur,
    Random& random
)
{
    ActionsCombat::executerAttaque(attaquant, defenseur, random);
}

bool GestionnaireTours::executerPotionSoin(
    Entite& entite,
    int soinPotion
)
{
    return ActionsCombat::executerPotionSoin(entite, soinPotion);
}

bool GestionnaireTours::executerPotionDegats(
    Entite& attaquant,
    Entite& defenseur,
    Random& random,
    int bonusPotionDegats
)
{
    return ActionsCombat::executerPotionDegats(
        attaquant,
        defenseur,
        random,
        bonusPotionDegats
    );
}