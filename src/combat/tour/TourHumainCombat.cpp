#include "combat/tour/TourHumainCombat.hpp"

#include "combat/ActionsCombat.hpp"
#include "combat/systeme/SystemeFuite.hpp"
#include "combat/systeme/SystemeObservation.hpp"

#include "core/Console.hpp"

#include "entite/Joueur.hpp"
#include "entite/Boss.hpp"

#include "interface/menu/MenuCombat.hpp"
#include "interface/menu/MenuEquipement.hpp"
#include "interface/menu/MenuInventaire.hpp"
#include "interface/menu/MenuPotionsCombat.hpp"

#include <iostream>

bool TourHumainCombat::jouer(
    Entite& attaquant,
    Entite& defenseur,
    Random& random,
    int soinPotion,
    int bonusPotionDegats
)
{
    (void)soinPotion;

    MenuCombat::afficherMenuTour(attaquant);

    int option = Console::demanderNombreEntre(
        0,
        7,
        "Choix invalide. Entre un chiffre entre 0 et 7."
    );

    Console::clear();

    if (option == 0)
    {
        return ouvrirInterfaceObservation(attaquant, defenseur);
    }

    if (option == 1)
    {
        ActionsCombat::executerAttaque(attaquant, defenseur, random);
        return true;
    }

    if (option == 2)
    {
        Joueur* joueur = dynamic_cast<Joueur*>(&attaquant);

        if (joueur == nullptr)
        {
            MenuCombat::afficherOptionNonDisponible();
            return false;
        }

        return MenuPotionsCombat::ouvrirSoinRapide(*joueur);
    }

    if (option == 3)
    {
        Joueur* joueur = dynamic_cast<Joueur*>(&attaquant);

        if (joueur == nullptr)
        {
            MenuCombat::afficherOptionNonDisponible();
            return false;
        }

        return MenuPotionsCombat::ouvrirContreCibleUnique(
            *joueur,
            defenseur,
            random,
            bonusPotionDegats
        );
    }

    if (option == 4)
    {
        Joueur* joueur = dynamic_cast<Joueur*>(&attaquant);

        if (joueur == nullptr)
        {
            MenuCombat::afficherOptionNonDisponible();
            return false;
        }

        MenuEquipement::ouvrir(*joueur);
        return false;
    }

    if (option == 5)
    {
        Joueur* joueur = dynamic_cast<Joueur*>(&attaquant);

        if (joueur == nullptr)
        {
            MenuCombat::afficherOptionNonDisponible();
            return false;
        }

        return MenuInventaire::ouvrir(*joueur);
    }

    if (option == 6)
    {
        std::cout << attaquant.getNom() << " baisse sa garde et passe son tour." << std::endl;
        std::cout << "Parfois, attendre le bon moment est déjà une décision." << std::endl;
        std::cout << std::endl;

        return true;
    }

    if (option == 7)
    {
        return gererFuite(attaquant, defenseur, random);
    }

    return false;
}

bool TourHumainCombat::ouvrirInterfaceObservation(
    Entite& joueurInterface,
    Entite& cible
)
{
    std::cout << "========== INTERFACE ==========" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Voir mes statistiques" << std::endl;
    std::cout << "2 : Inspecter l'adversaire" << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";

    int choix = Console::demanderNombreEntre(
        0,
        2,
        "Choix invalide. Entre 0, 1 ou 2."
    );

    Console::clear();

    if (choix == 0)
    {
        return false;
    }

    if (choix == 1)
    {
        joueurInterface.afficherStats();
        return false;
    }

    if (choix == 2)
    {
        SystemeObservation::afficherStatsTerminal(cible);
        return false;
    }

    return false;
}

bool TourHumainCombat::gererFuite(
    Entite& attaquant,
    Entite& defenseur,
    Random& random
)
{
    Boss* bossCible = dynamic_cast<Boss*>(&defenseur);

    if (bossCible != nullptr)
    {
        Joueur* joueur = dynamic_cast<Joueur*>(&attaquant);

        if (joueur != nullptr)
        {
            SystemeFuite::joueurTenteFuiteBoss(*joueur, *bossCible);
        }
        else
        {
            std::cout << "[la fuite est impossible durant ce combat]" << std::endl;
            std::cout << std::endl;
        }

        return true;
    }

    Joueur* joueur = dynamic_cast<Joueur*>(&attaquant);

    if (joueur == nullptr)
    {
        MenuCombat::afficherOptionNonDisponible();
        return false;
    }

    SystemeFuite::joueurTenteFuiteDuel(*joueur, defenseur, random);

    return true;
}