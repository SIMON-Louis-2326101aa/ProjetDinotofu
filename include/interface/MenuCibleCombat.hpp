#ifndef MENU_CIBLE_COMBAT_HPP
#define MENU_CIBLE_COMBAT_HPP

#include "entite/Joueur.hpp"
#include "combat/FileEnnemisCombat.hpp"
#include "core/Random.hpp"

class MenuCibleCombat
{
public:
    static bool ouvrirPourAttaque(
        Joueur& joueur,
        FileEnnemisCombat& vague,
        Random& random
    );

    static bool ouvrirPourPotionDegats(
        Joueur& joueur,
        FileEnnemisCombat& vague,
        Random& random,
        int bonusPotionDegats
    );

private:
    static int choisirCible(const FileEnnemisCombat& vague);

    static bool ouvrirMenuCible(
        Joueur& joueur,
        FileEnnemisCombat& vague,
        Random& random,
        bool attaqueAvecPotionDegats,
        int bonusPotionDegats
    );
};

#endif