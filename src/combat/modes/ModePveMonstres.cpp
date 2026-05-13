#include "combat/modes/ModePveMonstres.hpp"

#include "combat/GestionnaireTours.hpp"
#include "combat/SystemeFuite.hpp"
#include "combat/SystemeVagueCombat.hpp"

#include "core/Console.hpp"

#include "interface/MenuCombat.hpp"
#include "interface/MenuInventaire.hpp"
#include "interface/MenuEquipement.hpp"
#include "interface/MenuCibleCombat.hpp"
#include "interface/MenuPotionsCombat.hpp"

#include <iostream>

bool ModePveMonstres::jouerTourJoueur(
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

        if (choixInterface == 1)
        {
            joueur.afficherStats();
        }
        else if (choixInterface == 2)
        {
            vague.afficherEnnemisActifs();
            vague.afficherFileResume();
        }
        else if (choixInterface == 3)
        {
            vague.afficherFileResume();
        }

        return false;
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

void ModePveMonstres::jouerToursMonstres(
    Joueur& joueur,
    FileEnnemisCombat& vague,
    Random& random
)
{
    int i = 0;

    while (i < vague.getNombreEnnemisActifs())
    {
        if (joueur.estMort())
        {
            return;
        }

        if (!vague.indexActifValide(i))
        {
            ++i;
            continue;
        }

        Monstre& monstre = vague.getEnnemiActif(i);

        if (monstre.estMort())
        {
            ++i;
            continue;
        }

        if (SystemeFuite::monstreTenteFuite(monstre, random))
        {
            vague.retirerEnnemiActif(i);
            continue;
        }

        std::cout << "Tour de " << monstre.getNom() << std::endl;
        std::cout << std::endl;

        Console::pauseSecondes(1);

        GestionnaireTours::executerAttaque(monstre, joueur, random);

        Console::pauseSecondes(1);

        ++i;
    }

    vague.retirerMortsEtRemplacer();
}

void ModePveMonstres::lancer(Joueur& joueur, Random& random)
{
    SystemeVagueCombat::afficherIntroductionVague();

    FileEnnemisCombat vague = SystemeVagueCombat::creerVagueDemo();

    SystemeVagueCombat::afficherArriveePremiereLigne(vague);

    bool fuiteReussie = false;

    while (!joueur.estMort() && vague.aEncoreDesEnnemis() && !fuiteReussie)
    {
        bool tourJoueurTermine = false;

        while (!tourJoueurTermine && !joueur.estMort() && vague.aEncoreDesEnnemis() && !fuiteReussie)
        {
            tourJoueurTermine = jouerTourJoueur(
                joueur,
                vague,
                random,
                fuiteReussie
            );

            if (!tourJoueurTermine && !fuiteReussie)
            {
                std::cout << "Ton tour n'est pas encore consommé." << std::endl;
                std::cout << std::endl;
            }
        }

        if (!joueur.estMort() && vague.aEncoreDesEnnemis() && !fuiteReussie)
        {
            jouerToursMonstres(joueur, vague, random);
        }
    }

    if (fuiteReussie)
    {
        std::cout << "Tu as quitté le combat." << std::endl;
        std::cout << "Tu ne récupéreras qu'une partie des récompenses liées à ce qui s'est réellement passé." << std::endl;
        std::cout << std::endl;
        return;
    }

    if (joueur.estMort())
    {
        std::cout << joueur.getNom() << " tombe face à la vague ennemie." << std::endl;
        std::cout << "L'arène se referme dans un silence brutal." << std::endl;
        std::cout << std::endl;
        return;
    }

    std::cout << "Tous les monstres de la vague ont été vaincus." << std::endl;
    std::cout << joueur.getNom() << " reste debout au milieu des corps et de la poussière." << std::endl;
    std::cout << std::endl;
}