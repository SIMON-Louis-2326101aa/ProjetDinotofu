#include "combat/modes/pve/ModePveMonstres.hpp"

#include "combat/FileEnnemisCombat.hpp"
#include "combat/systeme/SystemeVagueCombat.hpp"

#include "combat/tour/vague/TourJoueurVagueCombat.hpp"
#include "combat/tour/vague/TourMonstresVagueCombat.hpp"

#include <iostream>

void ModePveMonstres::lancer(Joueur& joueur, Random& random)
{
    SystemeVagueCombat::afficherIntroductionVague();

    FileEnnemisCombat vague = SystemeVagueCombat::creerVagueDemo();

    SystemeVagueCombat::afficherArriveePremiereLigne(vague);

    bool fuiteReussie = false;

    while (!joueur.estMort() && vague.aEncoreDesEnnemis() && !fuiteReussie)
    {
        bool tourJoueurTermine = false;

        while (!tourJoueurTermine
            && !joueur.estMort()
            && vague.aEncoreDesEnnemis()
            && !fuiteReussie)
        {
            tourJoueurTermine = TourJoueurVagueCombat::jouer(
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
            TourMonstresVagueCombat::jouerToursMonstres(
                joueur,
                vague,
                random
            );
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