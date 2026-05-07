#include "core/Jeu.hpp"
#include "core/Console.hpp"
#include "classe/CatalogueClasses.hpp"
#include "combat/Combat.hpp"

#include <iostream>

Jeu::Jeu()
{
    nomJoueur = "";
    modeSelectionne = ModeJeu::PvPIA;
}

void Jeu::lancer()
{
    Console::clear();

    afficherIntroduction();
    demanderNomJoueur();
    choisirClasseJoueur();
    choisirModeDeJeu();
    afficherModeSelectionne();
    lancerModeSelectionne();
}

void Jeu::afficherIntroduction()
{
    std::cout << "Bonjour voyageur, et bienvenue dans Dinotofu." << std::endl;
    Console::pauseSecondes(1);

    std::cout << "Un monde de fantaisie, d'arènes et de baston," << std::endl;
    std::cout << "où chaque choix peut transformer un simple combattant en légende." << std::endl;
    std::cout << std::endl;

    Console::attendreEntree();
    Console::clear();
}

void Jeu::demanderNomJoueur()
{
    std::cout << "Quel est ton nom ?" << std::endl;
    std::cout << "> ";
    std::cin >> nomJoueur;

    std::cout << std::endl;
    std::cout << "Très bien, " << nomJoueur << "." << std::endl;
    std::cout << "L'arène se souviendra peut-être de ce nom..." << std::endl;
    std::cout << std::endl;

    Console::attendreEntree();
    Console::clear();
}

void Jeu::choisirClasseJoueur()
{
    std::cout << "Choisis ta classe et entre dans l'arène." << std::endl;
    std::cout << "Trois voies s'offrent à toi :" << std::endl;
    std::cout << std::endl;

    CatalogueClasses::afficherClassesDeBase();

    std::cout << "Veuillez entrer uniquement le chiffre correspondant." << std::endl;
    std::cout << "> ";

    int choix = Console::demanderNombreEntre(
        1,
        3,
        "Veuillez entrer un chiffre valide : 1, 2 ou 3."
    );

    ClasseJoueur classeChoisie = CatalogueClasses::creerClasseDeBase(choix);
    joueurPrincipal = Joueur(nomJoueur, classeChoisie);

    Console::clear();

    std::cout << nomJoueur << ", tu as choisi la classe : " << classeChoisie.getNom() << "." << std::endl;
    std::cout << "Tes statistiques ont été gravées dans l'arène avec succès." << std::endl;
    std::cout << std::endl;

    joueurPrincipal.afficherStats();

    Console::attendreEntree();
    Console::clear();
}

void Jeu::choisirModeDeJeu()
{
    std::cout << "Sélectionne le mode de jeu que tu veux lancer :" << std::endl;
    std::cout << std::endl;

    std::cout << "1 : PvP IA" << std::endl;
    std::cout << "    Affronte Matt, une IA simple mais prête à te casser les dents." << std::endl;
    std::cout << std::endl;

    std::cout << "2 : PvP 2 joueurs" << std::endl;
    std::cout << "    Deux combattants, une arène, aucune excuse." << std::endl;
    std::cout << std::endl;

    std::cout << "3 : PvE" << std::endl;
    std::cout << "    Une vague d'ennemis... enfin, quand les bugs arrêteront de fuir." << std::endl;
    std::cout << std::endl;

    std::cout << "4 : PvE Boss" << std::endl;
    std::cout << "    Une entité supérieure t'attend. Mauvaise idée ? Probablement." << std::endl;
    std::cout << std::endl;

    std::cout << "Veuillez entrer uniquement le chiffre correspondant." << std::endl;
    std::cout << "> ";

    int choix = Console::demanderNombreEntre(
        1,
        4,
        "Veuillez entrer un chiffre valide : 1, 2, 3 ou 4."
    );

    modeSelectionne = static_cast<ModeJeu>(choix);
}

void Jeu::afficherModeSelectionne()
{
    Console::clear();

    std::cout << "Mode sélectionné : ";

    switch (modeSelectionne)
    {
        case ModeJeu::PvPIA:
            std::cout << "PvP IA";
            break;

        case ModeJeu::PvPDeuxJoueurs:
            std::cout << "PvP 2 joueurs";
            break;

        case ModeJeu::PvE:
            std::cout << "PvE";
            break;

        case ModeJeu::PvEBoss:
            std::cout << "PvE Boss";
            break;
    }

    std::cout << std::endl;
    std::cout << std::endl;

    Console::attendreEntree();
    Console::clear();
}

void Jeu::lancerModeSelectionne()
{
    switch (modeSelectionne)
    {
        case ModeJeu::PvPIA:
        {
            Combat combat;
            combat.lancerPvpIA(joueurPrincipal);
            break;
        }

        case ModeJeu::PvPDeuxJoueurs:
        {
            Combat combat;
            combat.lancerPvpDeuxJoueurs(joueurPrincipal);
            break;
        }

        case ModeJeu::PvE:
            joueurPrincipal.afficherStats();
            std::cout << "Préparation de la vague d'ennemis..." << std::endl;
            Console::pauseSecondes(2);
            std::cout << "Ce mode n'est pas encore terminé." << std::endl;
            std::cout << "Pour l'instant, tout le monde a fui face à la puissance des bugs." << std::endl;
            break;

        case ModeJeu::PvEBoss:
        {
            Combat combat;
            combat.lancerPveBoss(joueurPrincipal);
            break;
        }
    }

    std::cout << std::endl;
}