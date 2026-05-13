#include "combat/Combat.hpp"

#include "combat/modes/ModePvp.hpp"
#include "combat/modes/ModePvpIA.hpp"
#include "combat/modes/ModePveBoss.hpp"

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