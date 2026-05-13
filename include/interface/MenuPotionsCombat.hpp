#ifndef MENU_POTIONS_COMBAT_HPP
#define MENU_POTIONS_COMBAT_HPP

#include "core/Random.hpp"

#include "entite/Joueur.hpp"
#include "entite/Entite.hpp"

#include "combat/FileEnnemisCombat.hpp"

#include "objet/consommable/TypeConsommable.hpp"

#include <vector>

class MenuPotionsCombat
{
public:
    static bool ouvrirSoinRapide(Joueur& joueur);

    static bool ouvrirContreCibleUnique(
        Joueur& joueur,
        Entite& cible,
        Random& random,
        int bonusPotionDegats
    );

    static bool ouvrirContreVague(
        Joueur& joueur,
        FileEnnemisCombat& vague,
        Random& random,
        int bonusPotionDegats
    );

private:
    static void afficherMenuPrincipal();

    static bool ouvrirCategorie(
        Joueur& joueur,
        TypeConsommable type,
        Entite* cible,
        FileEnnemisCombat* vague,
        Random& random,
        int bonusPotionDegats
    );

    static std::vector<int> recupererIndicesPotions(
        const Joueur& joueur,
        TypeConsommable type
    );

    static bool ouvrirSelectionPotion(
        Joueur& joueur,
        const std::vector<int>& indices,
        TypeConsommable type,
        Entite* cible,
        FileEnnemisCombat* vague,
        Random& random,
        int bonusPotionDegats
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

    static void afficherPotions(const Joueur& joueur);
    static void afficherPotionsFiltrees(
        const Joueur& joueur,
        const std::vector<int>& indices
    );

    static std::string typeVersTexte(TypeConsommable type);
};

#endif