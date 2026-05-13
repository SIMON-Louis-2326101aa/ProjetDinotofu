#include "interface/MenuInventaire.hpp"

#include "core/Console.hpp"

#include <iostream>

bool MenuInventaire::ouvrir(Joueur& joueur)
{
    bool inventaireOuvert = true;

    while (inventaireOuvert)
    {
        std::cout << "================ INVENTAIRE ================" << std::endl;
        std::cout << "Que veux-tu consulter ?" << std::endl;
        std::cout << std::endl;
        std::cout << "1 : Voir tout" << std::endl;
        std::cout << "2 : Voir les armes" << std::endl;
        std::cout << "3 : Voir les armures" << std::endl;
        std::cout << "4 : Voir les consommables" << std::endl;
        std::cout << "5 : Équipement simple" << std::endl;
        std::cout << "6 : Équipement détaillé" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "============================================" << std::endl;
        std::cout << "> ";

        int choixMenu = Console::demanderNombreEntre(
            0,
            6,
            "Choix invalide. Entre un chiffre entre 0 et 6."
        );

        Console::clear();

        if (choixMenu == 0)
        {
            return false;
        }

        if (choixMenu == 1)
        {
            joueur.getInventaire().afficher();
            continue;
        }

        if (choixMenu == 2)
        {
            ouvrirArmes(joueur);
            continue;
        }

        if (choixMenu == 3)
        {
            ouvrirArmures(joueur);
            continue;
        }

        if (choixMenu == 4)
        {
            bool tourConsomme = ouvrirConsommables(joueur);

            if (tourConsomme)
            {
                return true;
            }

            continue;
        }

        if (choixMenu == 5)
        {
            joueur.afficherEquipementSimple();
            continue;
        }

        if (choixMenu == 6)
        {
            joueur.afficherEquipementDetaille();
            continue;
        }
    }

    return false;
}

bool MenuInventaire::ouvrirArmes(Joueur& joueur)
{
    if (joueur.getInventaire().getNombreArmes() <= 0)
    {
        std::cout << "Tu n'as aucune arme dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    joueur.getInventaire().afficherListeArmes();

    std::cout << "Sélectionne une arme, ou entre -1 pour revenir." << std::endl;
    std::cout << "> ";

    int index = Console::demanderNombreEntre(
        -1,
        joueur.getInventaire().getNombreArmes() - 1,
        "Choix invalide. Entre un numéro d'arme valide, ou -1 pour revenir."
    );

    Console::clear();

    if (index == -1)
    {
        return false;
    }

    if (!joueur.getInventaire().possedeArme(index))
    {
        std::cout << "Cette arme n'existe pas dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    std::cout << "Que veux-tu faire avec cette arme ?" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;
    std::cout << "2 : Équiper" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "> ";

    int action = Console::demanderNombreEntre(
        0,
        2,
        "Choix invalide. Entre 0, 1 ou 2."
    );

    Console::clear();

    if (action == 1)
    {
        joueur.getInventaire().inspecterArme(index);
        return false;
    }

    if (action == 2)
    {
        if (joueur.equiperArme(index))
        {
            Arme arme = joueur.getArmeEquipee();

            std::cout << joueur.getNom() << " équipe : " << arme.getNom() << "." << std::endl;

            if (arme.estCassee())
            {
                std::cout << "Attention : cette arme est cassée, elle ne donnera aucun bonus." << std::endl;
            }

            std::cout << std::endl;
        }
        else
        {
            std::cout << "Impossible d'équiper cette arme." << std::endl;
            std::cout << std::endl;
        }

        return false;
    }

    return false;
}

bool MenuInventaire::ouvrirArmures(Joueur& joueur)
{
    if (joueur.getInventaire().getNombreArmures() <= 0)
    {
        std::cout << "Tu n'as aucune armure dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    joueur.getInventaire().afficherListeArmures();

    std::cout << "Sélectionne une armure, ou entre -1 pour revenir." << std::endl;
    std::cout << "> ";

    int index = Console::demanderNombreEntre(
        -1,
        joueur.getInventaire().getNombreArmures() - 1,
        "Choix invalide. Entre un numéro d'armure valide, ou -1 pour revenir."
    );

    Console::clear();

    if (index == -1)
    {
        return false;
    }

    if (!joueur.getInventaire().possedeArmure(index))
    {
        std::cout << "Cette armure n'existe pas dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    std::cout << "Que veux-tu faire avec cette armure ?" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;
    std::cout << "2 : Équiper" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "> ";

    int action = Console::demanderNombreEntre(
        0,
        2,
        "Choix invalide. Entre 0, 1 ou 2."
    );

    Console::clear();

    if (action == 1)
    {
        joueur.getInventaire().inspecterArmure(index);
        return false;
    }

    if (action == 2)
    {
        if (joueur.equiperArmure(index))
        {
            Armure armure = joueur.getArmureEquipee();

            std::cout << joueur.getNom() << " équipe : " << armure.getNom() << "." << std::endl;

            if (armure.estCassee())
            {
                std::cout << "Attention : cette armure est cassée, elle ne donnera aucun bonus." << std::endl;
            }
            else
            {
                std::cout << "Ses protections sont maintenant actives." << std::endl;
            }

            std::cout << std::endl;
        }
        else
        {
            std::cout << "Impossible d'équiper cette armure." << std::endl;
            std::cout << std::endl;
        }

        return false;
    }

    return false;
}

bool MenuInventaire::ouvrirConsommables(Joueur& joueur)
{
    if (joueur.getInventaire().getNombreConsommables() <= 0)
    {
        std::cout << "Tu n'as aucun consommable dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    joueur.getInventaire().afficherListeConsommables();

    std::cout << "Sélectionne un consommable, ou entre -1 pour revenir." << std::endl;
    std::cout << "> ";

    int index = Console::demanderNombreEntre(
        -1,
        joueur.getInventaire().getNombreConsommables() - 1,
        "Choix invalide. Entre un numéro de consommable valide, ou -1 pour revenir."
    );

    Console::clear();

    if (index == -1)
    {
        return false;
    }

    if (!joueur.getInventaire().possedeConsommable(index))
    {
        std::cout << "Ce consommable n'existe pas dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Consommable consommable = joueur.getInventaire().getConsommable(index);

    std::cout << "Que veux-tu faire avec ce consommable ?" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;

    if (consommable.getType() == TypeConsommable::Soin)
    {
        std::cout << "2 : Utiliser" << std::endl;
    }
    else
    {
        std::cout << "2 : Utiliser (indisponible ici)" << std::endl;
    }

    std::cout << "0 : Retour" << std::endl;
    std::cout << "> ";

    int action = Console::demanderNombreEntre(
        0,
        2,
        "Choix invalide. Entre 0, 1 ou 2."
    );

    Console::clear();

    if (action == 1)
    {
        joueur.getInventaire().inspecterConsommable(index);
        return false;
    }

    if (action == 2)
    {
        if (consommable.getType() != TypeConsommable::Soin)
        {
            std::cout << "Ce consommable demande une action spéciale." << std::endl;
            std::cout << "Pour une potion de rage, utilise l'option 3 du menu de combat." << std::endl;
            std::cout << std::endl;
            return false;
        }

        joueur.soigner(consommable.getPuissance());
        joueur.getInventaire().retirerConsommable(index);

        std::cout << joueur.getNom() << " utilise : " << consommable.getNom() << "." << std::endl;
        std::cout << "Ses blessures se referment, et il récupère "
                  << consommable.getPuissance() << " PV." << std::endl;
        std::cout << joueur.getNom() << " possède maintenant "
                  << joueur.getPv() << "/" << joueur.getPvMax() << " PV." << std::endl;
        std::cout << std::endl;

        return true;
    }

    return false;
}