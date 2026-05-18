#include "combat/modes/boss/ModePveBoss.hpp"

#include "combat/GestionnaireTours.hpp"

#include "core/Console.hpp"
#include "classe/CatalogueClasses.hpp"
#include "boss/CatalogueBoss.hpp"
#include "interface/AffichageCombat.hpp"

#include <iostream>

void ModePveBoss::lancer(Joueur& joueur1, Random& random)
{
    std::cout << "Vous sentez une aura maléfique autour de vous." << std::endl;
    Console::pauseSecondes(2);

    std::cout << "Par chance, votre âme est encore pure." << std::endl;
    std::cout << "À mesure que vous avancez, votre force change, comme si l'arène refusait de vous laisser mourir trop vite." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(2);

    ClasseJoueur classeEvoluee = CatalogueClasses::creerClasseEvolueeDepuisClasse(joueur1.getType());
    joueur1.appliquerClasse(classeEvoluee);

    std::cout << joueur1.getNom() << ", ta classe évolue en : " << joueur1.getType() << "." << std::endl;
    std::cout << "Tes PV et tes objets ont été renforcés pour ce combat." << std::endl;
    std::cout << "Tes dégâts, eux, restent bloqués : même l'arène semble avoir ses limites." << std::endl;
    std::cout << std::endl;

    joueur1.afficherStats();

    Console::pauseSecondes(3);

    std::cout << joueur1.getNom() << ", choisis le type d'apparition du boss :" << std::endl;
    std::cout << std::endl;
    std::cout << "1 : Boss aléatoire" << std::endl;
    std::cout << "2 : Choisir le boss toi-même" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";

    int choixTypeBoss = Console::demanderNombreEntre(
        1,
        2,
        "Veuillez entrer un chiffre valide : 1 ou 2."
    );

    int choixBoss;

    if (choixTypeBoss == 1)
    {
        choixBoss = random.entre(1, 3);
    }
    else
    {
        Console::clear();

        std::cout << "Sélectionne l'entité que tu veux exterminer :" << std::endl;
        std::cout << std::endl;

        CatalogueBoss::afficherBossDisponibles();

        std::cout << "Leurs statistiques resteront inconnues pour le moment." << std::endl;
        std::cout << "Tu auras besoin d'un bon instinct... ou d'une chance insolente." << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

        choixBoss = Console::demanderNombreEntre(
            1,
            3,
            "Veuillez entrer un chiffre valide : 1, 2 ou 3."
        );
    }

    Boss boss = CatalogueBoss::creerBoss(choixBoss);

    Console::clear();

    std::cout << "Préparation du boss..." << std::endl;
    Console::pauseSecondes(2);

    std::cout << boss.getNom() << " est entré dans l'arène." << std::endl;
    std::cout << boss.getNom() << " est de type : " << boss.getType() << "." << std::endl;
    std::cout << "Fais attention : ce combat pourrait vraiment te coûter cher." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(3);

    int tour = random.choisirPremierTour();

    std::cout << "Prépare-toi..." << std::endl;
    Console::pauseSecondes(2);
    std::cout << "Le combat contre " << boss.getNom() << " commence maintenant." << std::endl;
    std::cout << std::endl;

    while (!joueur1.estMort() && !boss.estMort())
    {
        bool tourTermine = false;

        if (tour == 1)
        {
            tourTermine = GestionnaireTours::jouerTourHumain(
                joueur1,
                boss,
                random,
                SOIN_POTION_BOSS,
                BONUS_POTION_DEGATS_BOSS
            );

            if (tourTermine)
            {
                GestionnaireTours::verifierDecryptageBoss(boss);
                boss.reduireDelaiUltime();
                tour = 2;
            }
        }
        else
        {
            tourTermine = GestionnaireTours::jouerTourBoss(
                boss,
                joueur1,
                random
            );

            if (tourTermine)
            {
                tour = 1;
            }
        }
    }

    AffichageCombat::afficherResultatCombat(joueur1, boss);
}