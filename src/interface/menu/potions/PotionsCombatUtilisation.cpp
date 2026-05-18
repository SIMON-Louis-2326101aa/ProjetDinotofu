#include "interface/menu/potions/PotionsCombatUtilisation.hpp"

#include "combat/ActionsCombat.hpp"

#include "combat/action/AttaqueCombat.hpp"

#include "interface/menu/MenuCibleCombat.hpp"

#include "objet/Inventaire.hpp"

#include <iostream>

bool PotionsCombatUtilisation::utiliserPotionSoin(
    Joueur& joueur,
    int indexConsommable,
    const Consommable& potion
)
{
    if (!joueur.getInventaire().possedeConsommable(indexConsommable))
    {
        std::cout << "Cette potion n'est plus disponible." << std::endl;
        std::cout << std::endl;
        return false;
    }

    joueur.soigner(potion.getPuissance());
    joueur.getInventaire().retirerConsommable(indexConsommable);

    std::cout << joueur.getNom()
              << " boit "
              << potion.getNom()
              << " et récupère "
              << potion.getPuissance()
              << " PV."
              << std::endl;
    std::cout << std::endl;

    return true;
}

bool PotionsCombatUtilisation::utiliserPotionSelectionnee(
    Joueur& joueur,
    int indexConsommable,
    TypeConsommable type,
    Entite* cible,
    FileEnnemisCombat* vague,
    Random& random,
    int bonusPotionDegats
)
{
    if (!joueur.getInventaire().possedeConsommable(indexConsommable))
    {
        std::cout << "Cette potion n'est plus disponible." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Consommable potion = joueur.getInventaire().getConsommable(indexConsommable);

    if (type == TypeConsommable::Soin)
    {
        return utiliserPotionSoin(joueur, indexConsommable, potion);
    }

    if (type == TypeConsommable::Degats)
    {
        int bonusTotal = potion.getPuissance();

        if (bonusPotionDegats > 0)
        {
            bonusTotal = potion.getPuissance();
        }

        if (vague != nullptr)
        {
            bool attaqueLancee = MenuCibleCombat::ouvrirPourAttaqueBoostee(
                joueur,
                *vague,
                random,
                bonusTotal
            );

            if (attaqueLancee)
            {
                joueur.getInventaire().retirerConsommable(indexConsommable);
            }

            return attaqueLancee;
        }

        if (cible != nullptr)
        {
            joueur.getInventaire().retirerConsommable(indexConsommable);

            std::cout << joueur.getNom()
                      << " utilise "
                      << potion.getNom()
                      << "."
                      << std::endl;
            std::cout << std::endl;

            AttaqueCombat::executerAttaqueBoostee(
                joueur,
                *cible,
                random,
                bonusTotal
            );

            return true;
        }

        std::cout << "Aucune cible offensive disponible." << std::endl;
        std::cout << std::endl;
        return false;
    }

    if (type == TypeConsommable::Buff || type == TypeConsommable::Debuff)
    {
        std::cout << "[cette option n'est pas encore accessible pour ce mode]" << std::endl;
        std::cout << "La potion existe, mais son effet n'est pas encore codé." << std::endl;
        std::cout << std::endl;
        return false;
    }

    std::cout << "[cette option n'est pas encore accessible pour ce mode]" << std::endl;
    std::cout << std::endl;

    return false;
}