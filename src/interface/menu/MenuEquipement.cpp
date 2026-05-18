#include "interface/menu/MenuEquipement.hpp"

#include "core/Console.hpp"

#include "interface/menu/equipement/EquipementAffichage.hpp"
#include "interface/menu/equipement/EquipementComparaison.hpp"

#include <iostream>

bool MenuEquipement::ouvrir(Joueur& joueur)
{
    while (true)
    {
        EquipementAffichage::afficherMenuPrincipal();

        int choix = Console::demanderNombreEntre(
            0,
            4,
            "Choix invalide. Entre un chiffre entre 0 et 4."
        );

        Console::clear();

        if (choix == 0)
        {
            return false;
        }

        if (choix == 1)
        {
            joueur.afficherEquipementSimple();
            continue;
        }

        if (choix == 2)
        {
            joueur.afficherEquipementDetaille();
            continue;
        }

        if (choix == 3)
        {
            equiperArmeDepuisInventaire(joueur);
            continue;
        }

        if (choix == 4)
        {
            equiperArmureDepuisInventaire(joueur);
            continue;
        }
    }
}

bool MenuEquipement::equiperArmeDepuisInventaire(Joueur& joueur)
{
    if (joueur.getInventaire().getNombreArmes() <= 0)
    {
        std::cout << joueur.getNom() << " n'a aucune arme à équiper." << std::endl;
        std::cout << std::endl;
        return false;
    }

    EquipementAffichage::afficherListeArmes(joueur);

    int choix = Console::demanderNombreEntre(
        -1,
        joueur.getInventaire().getNombreArmes() - 1,
        "Choix invalide. Entre un numéro d'arme valide, ou -1 pour revenir."
    );

    Console::clear();

    if (choix == -1)
    {
        return false;
    }

    if (!joueur.getInventaire().possedeArme(choix))
    {
        std::cout << "Cette arme n'existe pas dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Arme nouvelleArme = joueur.getInventaire().getArme(choix);

    EquipementAffichage::afficherArmeSelectionnee(nouvelleArme);

    int action = Console::demanderNombreEntre(
        0,
        3,
        "Choix invalide. Entre un chiffre entre 0 et 3."
    );

    Console::clear();

    if (action == 0)
    {
        return false;
    }

    if (action == 1)
    {
        joueur.getInventaire().inspecterArme(choix);
        return false;
    }

    if (action == 2)
    {
        EquipementComparaison::afficherComparaisonArme(joueur, nouvelleArme);
        return false;
    }

    if (!joueur.equiperArme(choix))
    {
        std::cout << "Impossible d'équiper cette arme." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Arme armeEquipee = joueur.getArmeEquipee();

    std::cout << joueur.getNom() << " équipe : " << armeEquipee.getNom() << "." << std::endl;

    if (armeEquipee.estCassee())
    {
        std::cout << "Attention : cette arme est cassée, elle ne donnera aucun bonus." << std::endl;
    }
    else
    {
        std::cout << "La prise en main est bonne. Cette arme est prête au combat." << std::endl;
    }

    std::cout << std::endl;

    return false;
}

bool MenuEquipement::equiperArmureDepuisInventaire(Joueur& joueur)
{
    if (joueur.getInventaire().getNombreArmures() <= 0)
    {
        std::cout << joueur.getNom() << " n'a aucune armure à équiper." << std::endl;
        std::cout << std::endl;
        return false;
    }

    EquipementAffichage::afficherListeArmures(joueur);

    int choix = Console::demanderNombreEntre(
        -1,
        joueur.getInventaire().getNombreArmures() - 1,
        "Choix invalide. Entre un numéro d'armure valide, ou -1 pour revenir."
    );

    Console::clear();

    if (choix == -1)
    {
        return false;
    }

    if (!joueur.getInventaire().possedeArmure(choix))
    {
        std::cout << "Cette armure n'existe pas dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Armure nouvelleArmure = joueur.getInventaire().getArmure(choix);

    EquipementAffichage::afficherArmureSelectionnee(nouvelleArmure);

    int action = Console::demanderNombreEntre(
        0,
        3,
        "Choix invalide. Entre un chiffre entre 0 et 3."
    );

    Console::clear();

    if (action == 0)
    {
        return false;
    }

    if (action == 1)
    {
        joueur.getInventaire().inspecterArmure(choix);
        return false;
    }

    if (action == 2)
    {
        EquipementComparaison::afficherComparaisonArmure(joueur, nouvelleArmure);
        return false;
    }

    if (!joueur.equiperArmure(choix))
    {
        std::cout << "Impossible d'équiper cette armure." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Armure armureEquipee = joueur.getArmureEquipee();

    std::cout << joueur.getNom() << " équipe : " << armureEquipee.getNom() << "." << std::endl;

    if (armureEquipee.estCassee())
    {
        std::cout << "Attention : cette armure est cassée, elle ne donnera aucun bonus." << std::endl;
    }
    else
    {
        std::cout << "Ses protections sont maintenant actives." << std::endl;
    }

    std::cout << joueur.getNom() << " possède maintenant "
              << joueur.getPv()
              << "/"
              << joueur.getPvMax()
              << " PV."
              << std::endl;

    std::cout << std::endl;

    return false;
}