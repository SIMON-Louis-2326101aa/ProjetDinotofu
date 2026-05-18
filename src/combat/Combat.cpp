#include "combat/Combat.hpp"

#include "combat/modes/pvp/ModePvp.hpp"
#include "combat/modes/pvp/ModePvpIA.hpp"
#include "combat/modes/boss/ModePveBoss.hpp"
#include "combat/modes/pve/ModePveMonstres.hpp"

void Combat::lancerPvpDeuxJoueurs(Joueur& joueur1)
{
    ModePvp::lancer(joueur1, random);
}

void Combat::lancerPvpIA(Joueur& joueur1)
{
    ModePvpIA::lancer(joueur1, random);
}

void Combat::lancerPveBoss(Joueur& joueur1)
{
    ModePveBoss::lancer(joueur1, random);
}

void Combat::lancerPveMonstres(Joueur& joueur1)
{
    ModePveMonstres::lancer(joueur1, random);
}