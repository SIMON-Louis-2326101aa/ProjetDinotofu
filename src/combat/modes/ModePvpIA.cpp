#include "combat/modes/ModePvpIA.hpp"

#include "combat/GestionnaireTours.hpp"

#include "core/Console.hpp"
#include "classe/CatalogueClasses.hpp"
#include "interface/AffichageCombat.hpp"

#include <iostream>

void ModePvpIA::lancer(Joueur& joueur1, Random& random)
{
    std::cout << "Préparation de l'IA..." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(1);

    std::cout << "Matt est entré dans l'arène." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(1);

    std::cout << joueur1.getNom() << ", choisis comment Matt obtiendra sa classe :" << std::endl;
    std::cout << std::endl;
    std::cout << "1 : Classe aléatoire" << std::endl;
    std::cout << "2 : Choisir sa classe toi-même" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";

    int choixTypeClasse = Console::demanderNombreEntre(
        1,
        2,
        "Veuillez entrer un chiffre valide : 1 ou 2."
    );

    int choixClasseIA;

    if (choixTypeClasse == 1)
    {
        choixClasseIA = random.entre(1, 3);
    }
    else
    {
        Console::clear();

        std::cout << "Choisis la classe que Matt devra utiliser." << std::endl;
        std::cout << "Pas très fair-play, mais l'arène accepte ce genre de petit caprice." << std::endl;
        std::cout << std::endl;

        CatalogueClasses::afficherClassesDeBase();

        std::cout << "Veuillez entrer uniquement le chiffre correspondant." << std::endl;
        std::cout << "> ";

        choixClasseIA = Console::demanderNombreEntre(
            1,
            3,
            "Veuillez entrer un chiffre valide : 1, 2 ou 3."
        );
    }

    Joueur ia("Matt", CatalogueClasses::creerClasseDeBase(choixClasseIA));
    ia.initialiserInventaireDeBase();

    Console::clear();

    std::cout << ia.getNom() << " a choisi la classe : " << ia.getType() << "." << std::endl;
    std::cout << "Ses statistiques ont été gravées dans l'arène." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(2);

    int tour = random.choisirPremierTour();

    std::cout << "Prépare-toi..." << std::endl;
    Console::pauseSecondes(2);
    std::cout << "Le combat contre " << ia.getNom() << " commence maintenant." << std::endl;
    std::cout << std::endl;

    while (!joueur1.estMort() && !ia.estMort())
    {
        bool tourTermine = false;

        if (tour == 1)
        {
            tourTermine = GestionnaireTours::jouerTourHumain(
                joueur1,
                ia,
                random,
                SOIN_POTION,
                BONUS_POTION_DEGATS
            );

            if (tourTermine)
            {
                tour = 2;
            }
        }
        else
        {
            tourTermine = GestionnaireTours::jouerTourIA(
                ia,
                joueur1,
                random,
                SOIN_POTION,
                BONUS_POTION_DEGATS
            );

            if (tourTermine)
            {
                tour = 1;
            }
        }
    }

    AffichageCombat::afficherResultatCombat(joueur1, ia);
}