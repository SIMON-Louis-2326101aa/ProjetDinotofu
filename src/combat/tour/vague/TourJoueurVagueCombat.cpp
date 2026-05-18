#include "combat/tour/vague/TourJoueurVagueCombat.hpp"

#include "combat/systeme/SystemeFuite.hpp"

#include "core/Console.hpp"

#include "interface/menu/MenuCombat.hpp"
#include "interface/menu/MenuCibleCombat.hpp"
#include "interface/menu/MenuEquipement.hpp"
#include "interface/menu/MenuInventaire.hpp"
#include "interface/menu/MenuPotionsCombat.hpp"

#include <iostream>

bool TourJoueurVagueCombat::jouer(
    Joueur& joueur,
    FileEnnemisCombat& vague,
    Random& random,
    bool& fuiteReussie
)
{
    MenuCombat::afficherMenuTour(joueur);

    int choix = Console::demanderNombreEntre(
        0,
        7,
        "Choix invalide. Entre un chiffre entre 0 et 7."
    );

    Console::clear();

    if (choix == 0)
    {
        return ouvrirInterfaceVague(joueur, vague);
    }

    if (choix == 1)
    {
        return MenuCibleCombat::ouvrirPourAttaque(
            joueur,
            vague,
            random
        );
    }

    if (choix == 2)
    {
        return MenuPotionsCombat::ouvrirSoinRapide(joueur);
    }

    if (choix == 3)
    {
        return MenuPotionsCombat::ouvrirContreVague(
            joueur,
            vague,
            random,
            BONUS_POTION_DEGATS_PVE
        );
    }

    if (choix == 4)
    {
        MenuEquipement::ouvrir(joueur);
        return false;
    }

    if (choix == 5)
    {
        return MenuInventaire::ouvrir(joueur);
    }

    if (choix == 6)
    {
        std::cout << joueur.getNom() << " choisit de ne rien faire ce tour-ci." << std::endl;
        std::cout << "Parfois, survivre commence par attendre le bon moment." << std::endl;
        std::cout << std::endl;

        return true;
    }

    if (choix == 7)
    {
        fuiteReussie = SystemeFuite::joueurTenteFuite(joueur, random);
        return true;
    }

    return false;
}

bool TourJoueurVagueCombat::ouvrirInterfaceVague(
    Joueur& joueur,
    FileEnnemisCombat& vague
)
{
    std::cout << "========== INTERFACE ==========" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Voir mes statistiques" << std::endl;
    std::cout << "2 : Inspecter les adversaires" << std::endl;
    std::cout << "3 : Voir l'état du combat" << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";

    int choixInterface = Console::demanderNombreEntre(
        0,
        3,
        "Choix invalide. Entre un chiffre entre 0 et 3."
    );

    Console::clear();

    if (choixInterface == 0)
    {
        return false;
    }

    if (choixInterface == 1)
    {
        joueur.afficherStats();
        return false;
    }

    if (choixInterface == 2)
    {
        vague.afficherEnnemisActifs();
        vague.afficherFileResume();
        return false;
    }

    if (choixInterface == 3)
    {
        vague.afficherFileResume();
        return false;
    }

    return false;
}