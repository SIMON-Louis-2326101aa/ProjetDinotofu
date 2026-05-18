#ifndef POTIONS_COMBAT_AFFICHAGE_HPP
#define POTIONS_COMBAT_AFFICHAGE_HPP

#include "entite/Joueur.hpp"
#include "objet/consommable/Consommable.hpp"

#include <vector>

class PotionsCombatAffichage
{
public:
    static void afficherMenuPrincipal();

    static void afficherSoinRapide(
        const Joueur& joueur,
        const std::vector<int>& indices
    );

    static void afficherPotionSoinSelectionnee(const Consommable& potion);
    static void afficherPotionSelectionnee(const Consommable& potion);

    static void afficherPotions(const Joueur& joueur);

    static void afficherPotionsFiltrees(
        const Joueur& joueur,
        const std::vector<int>& indices
    );
};

#endif