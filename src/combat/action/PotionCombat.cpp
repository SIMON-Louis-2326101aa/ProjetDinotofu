#include "combat/action/PotionCombat.hpp"

#include "combat/action/AttaqueCombat.hpp"

#include "core/Console.hpp"

#include "entite/Joueur.hpp"

#include "objet/consommable/Consommable.hpp"
#include "objet/consommable/TypeConsommable.hpp"

#include <iostream>

bool PotionCombat::executerPotionSoin(
    Entite& entite,
    int soinPotion
)
{
    Joueur* joueur = dynamic_cast<Joueur*>(&entite);

    if (joueur != nullptr)
    {
        Consommable potion;

        if (!joueur->getInventaire().utiliserPremierConsommable(
            TypeConsommable::Soin,
            potion
        ))
        {
            std::cout << joueur->getNom()
                      << " fouille son inventaire..."
                      << std::endl;
            std::cout << "Mais aucune potion de soin n'est disponible." << std::endl;
            std::cout << "Il peut encore tenter autre chose." << std::endl;
            std::cout << std::endl;

            return false;
        }

        joueur->soigner(potion.getPuissance());

        std::cout << joueur->getNom()
                  << " utilise : "
                  << potion.getNom()
                  << "."
                  << std::endl;

        std::cout << "Ses blessures se referment, et il récupère "
                  << potion.getPuissance()
                  << " PV."
                  << std::endl;

        std::cout << joueur->getNom()
                  << " possède maintenant "
                  << joueur->getPv()
                  << "/"
                  << joueur->getPvMax()
                  << " PV."
                  << std::endl;

        std::cout << std::endl;

        return true;
    }

    if (entite.utiliserPotionSoin(soinPotion))
    {
        std::cout << entite.getNom()
                  << " utilise une potion de soin."
                  << std::endl;
        std::cout << "Sa vitalité revient lentement." << std::endl;
        std::cout << std::endl;

        return true;
    }

    std::cout << entite.getNom()
              << " n'a plus aucune potion de soin."
              << std::endl;
    std::cout << std::endl;

    return false;
}

bool PotionCombat::executerPotionDegats(
    Entite& attaquant,
    Entite& defenseur,
    Random& random,
    int bonusPotionDegats
)
{
    int bonusUtilise = bonusPotionDegats;

    Joueur* joueur = dynamic_cast<Joueur*>(&attaquant);

    if (joueur != nullptr)
    {
        Consommable potion;

        if (!joueur->getInventaire().utiliserPremierConsommable(
            TypeConsommable::Degats,
            potion
        ))
        {
            std::cout << joueur->getNom()
                      << " cherche une potion de rage dans son inventaire..."
                      << std::endl;
            std::cout << "Mais aucune potion offensive n'est disponible." << std::endl;
            std::cout << "Il peut encore tenter autre chose." << std::endl;
            std::cout << std::endl;

            return false;
        }

        bonusUtilise = potion.getPuissance();

        std::cout << joueur->getNom()
                  << " utilise : "
                  << potion.getNom()
                  << "."
                  << std::endl;
    }
    else
    {
        if (!attaquant.consommerPotionDegats())
        {
            std::cout << attaquant.getNom()
                      << " cherche une potion offensive, mais sa rage est déjà épuisée."
                      << std::endl;
            std::cout << "Il peut encore tenter autre chose." << std::endl;
            std::cout << std::endl;

            return false;
        }
    }

    std::cout << attaquant.getNom()
              << " sent ses forces monter d'un coup."
              << std::endl;
    std::cout << "Une rage brutale s'empare de lui..." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(1);

    AttaqueCombat::executerAttaqueBoostee(
        attaquant,
        defenseur,
        random,
        bonusUtilise
    );

    return true;
}