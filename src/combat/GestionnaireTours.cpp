#include "combat/GestionnaireTours.hpp"

#include "combat/ActionsCombat.hpp"
#include "combat/BossCombat.hpp"
#include "combat/IACombat.hpp"
#include "combat/SystemeObservation.hpp"
#include "combat/SystemeFuite.hpp"

#include "core/Console.hpp"

#include "interface/MenuCombat.hpp"
#include "interface/MenuPotionsCombat.hpp"
#include "interface/MenuEquipement.hpp"
#include "interface/MenuInventaire.hpp"

#include <iostream>

bool GestionnaireTours::jouerTourHumain(
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

        std::cout << "[cette option n'est pas encore accessible pour ce mode]" << std::endl;
        std::cout << "La fuite en duel sera gérée plus tard avec le poids, le niveau et les armes à distance." << std::endl;
        std::cout << std::endl;

        return false;
    }

    return false;
}

bool GestionnaireTours::jouerTourIA(
    Entite& ia,
    Entite& defenseur,
    Random& random,
    int soinPotion,
    int bonusPotionDegats
)
{
    std::cout << "Tour de " << ia.getNom() << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(1);

    int option = IACombat::choisirActionIA(ia, random);

    if (option == 0 || option == 1)
    {
        ActionsCombat::executerAttaque(ia, defenseur, random);
        return true;
    }

    if (option == 2)
    {
        return ActionsCombat::executerPotionSoin(ia, soinPotion);
    }

    if (option == 3)
    {
        return ActionsCombat::executerPotionDegats(
            ia,
            defenseur,
            random,
            bonusPotionDegats
        );
    }

    std::cout << ia.getNom() << " hésite, fixe le vide, et passe son tour." << std::endl;
    std::cout << std::endl;

    return true;
}

bool GestionnaireTours::jouerTourBoss(
    Boss& boss,
    Entite& joueur,
    Random& random
)
{
    std::cout << "Tour de " << boss.getNom() << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(1);

    int option = IACombat::choisirActionBoss(boss, random);

    if (option == 0 || option == 1)
    {
        ActionsCombat::executerAttaque(boss, joueur, random);
        return BossCombat::gererFinTourBoss(boss, joueur);
    }

    if (option == 2)
    {
        if (boss.utiliserPotionSoin(boss.getPvMax() * 10 / 100))
        {
            std::cout << boss.getNom() << " récupère une partie de sa vitalité." << std::endl;
            std::cout << std::endl;
        }

        return BossCombat::gererFinTourBoss(boss, joueur);
    }

    if (option == 3)
    {
        ActionsCombat::executerPotionDegats(boss, joueur, random, 50);
        return BossCombat::gererFinTourBoss(boss, joueur);
    }

    if (option == 4)
    {
        BossCombat::executerUltimeBoss(boss, joueur, random);
        return BossCombat::gererFinTourBoss(boss, joueur);
    }

    std::cout << boss.getNom()
              << " reste immobile, comme s'il observait déjà ta fin."
              << std::endl;
    std::cout << std::endl;

    return BossCombat::gererFinTourBoss(boss, joueur);
}

bool GestionnaireTours::ouvrirInterfaceObservation(
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

void GestionnaireTours::verifierDecryptageBoss(Boss& boss)
{
    BossCombat::verifierDecryptageBoss(boss);
}

void GestionnaireTours::executerAttaque(
    Entite& attaquant,
    Entite& defenseur,
    Random& random
)
{
    ActionsCombat::executerAttaque(attaquant, defenseur, random);
}

bool GestionnaireTours::executerPotionSoin(
    Entite& entite,
    int soinPotion
)
{
    return ActionsCombat::executerPotionSoin(entite, soinPotion);
}

bool GestionnaireTours::executerPotionDegats(
    Entite& attaquant,
    Entite& defenseur,
    Random& random,
    int bonusPotionDegats
)
{
    return ActionsCombat::executerPotionDegats(
        attaquant,
        defenseur,
        random,
        bonusPotionDegats
    );
}