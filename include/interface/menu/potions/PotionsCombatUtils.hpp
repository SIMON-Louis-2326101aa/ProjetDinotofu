#ifndef POTIONS_COMBAT_UTILS_HPP
#define POTIONS_COMBAT_UTILS_HPP

#include "entite/Joueur.hpp"
#include "objet/consommable/TypeConsommable.hpp"

#include <string>
#include <vector>

class PotionsCombatUtils
{
public:
    static std::vector<int> recupererIndicesPotions(
        const Joueur& joueur,
        TypeConsommable type
    );

    static std::string typeVersTexte(TypeConsommable type);
};

#endif