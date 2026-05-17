#include "combat/modes/ModePvp.hpp"

#include "combat/GestionnaireTours.hpp"

#include "core/Console.hpp"
#include "classe/CatalogueClasses.hpp"
#include "interface/AffichageCombat.hpp"

#include <iostream>

void ModePvp::lancer(Joueur& joueur1, Random& random)
{
    std::cout << "À votre tour, Joueur 2." << std::endl;
    std::cout << "Quel est ton nom ?" << std::endl;
    std::cout << "> ";

    std::string nomJoueur2;
    std::cin >> nomJoueur2;

    Console::clear();

    std::cout << "Très bien, " << nomJoueur2 << "." << std::endl;
    std::cout << "Choisis ta classe et entre dans l'arène." << std::endl;
    std::cout << std::endl;

    CatalogueClasses::afficherClassesDeBase();

    std::cout << "Veuillez entrer uniquement le chiffre correspondant." << std::endl;
    std::cout << "> ";

    int choixClasse = Console::demanderNombreEntre(
        1,
        3,
        "Veuillez entrer un chiffre valide : 1, 2 ou 3."
    );

    Joueur joueur2(nomJoueur2, CatalogueClasses::creerClasseDeBase(choixClasse));
    joueur2.initialiserInventaireDeBase();

    Console::clear();

    std::cout << joueur2.getNom() << ", tes statistiques ont été gravées dans l'arène." << std::endl;
    std::cout << "Le duel peut maintenant commencer." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(2);

    int tour = random.choisirPremierTour();

    std::cout << "Préparez-vous..." << std::endl;
    Console::pauseSecondes(2);
    std::cout << "Le combat commence maintenant." << std::endl;
    std::cout << std::endl;

    while (!joueur1.estMort() && !joueur2.estMort())
    {
        bool tourTermine = false;

        if (tour == 1)
        {
            tourTermine = GestionnaireTours::jouerTourHumain(
                joueur1,
                joueur2,
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
            tourTermine = GestionnaireTours::jouerTourHumain(
                joueur2,
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

    AffichageCombat::afficherResultatCombat(joueur1, joueur2);
}