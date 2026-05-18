#ifndef POTIONS_COMBAT_UTILISATION_HPP
#define POTIONS_COMBAT_UTILISATION_HPP

#include "core/Random.hpp"

#include "entite/Joueur.hpp"
#include "entite/Entite.hpp"

#include "combat/FileEnnemisCombat.hpp"

#include "objet/consommable/Consommable.hpp"
#include "objet/consommable/TypeConsommable.hpp"

class PotionsCombatUtilisation
{
public:
    static bool utiliserPotionSoin(
        Joueur& joueur,
        int indexConsommable,
        const Consommable& potion
    );

    static bool utiliserPotionSelectionnee(
        Joueur& joueur,
        int indexConsommable,
        TypeConsommable type,
        Entite* cible,
        FileEnnemisCombat* vague,
        Random& random,
        int bonusPotionDegats
    );
};

#endif