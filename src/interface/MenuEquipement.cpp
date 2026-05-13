#include "interface/MenuEquipement.hpp"

#include "core/Console.hpp"

#include <iostream>

bool MenuEquipement::ouvrir(Joueur& joueur)
{
    bool menuOuvert = true;

    while (menuOuvert)
    {
        std::cout << "================ ÉQUIPEMENT ================" << std::endl;
        std::cout << "Que veux-tu faire ?" << std::endl;
        std::cout << std::endl;
        std::cout << "1 : Affichage simple" << std::endl;
        std::cout << "2 : Affichage détaillé" << std::endl;
        std::cout << "3 : Équiper une arme" << std::endl;
        std::cout << "4 : Équiper une armure" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "============================================" << std::endl;
        std::cout << "> ";

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

    return false;
}

void MenuEquipement::afficherComparaisonArme(const Joueur& joueur, const Arme& nouvelleArme)
{
    std::cout << "=========== COMPARAISON D'ARME ===========" << std::endl;

    if (joueur.aUneArmeEquipee())
    {
        Arme armeActuelle = joueur.getArmeEquipee();

        std::cout << "Arme actuelle : " << armeActuelle.getNom() << std::endl;
        std::cout << "Dégâts bonus : +"
                  << armeActuelle.getBonusDegatsMin()
                  << " à +"
                  << armeActuelle.getBonusDegatsMax()
                  << std::endl;
        std::cout << "Critique bonus : +" << armeActuelle.getBonusCritique() << std::endl;

        if (armeActuelle.estIndestructible())
        {
            std::cout << "Durabilité : Indestructible" << std::endl;
        }
        else
        {
            std::cout << "Durabilité : "
                      << armeActuelle.getDurabilite()
                      << "/"
                      << armeActuelle.getDurabiliteMax()
                      << std::endl;
        }

        std::cout << std::endl;
    }
    else
    {
        std::cout << "Arme actuelle : Aucune" << std::endl;
        std::cout << std::endl;
    }

    std::cout << "Nouvelle arme : " << nouvelleArme.getNom() << std::endl;
    std::cout << "Dégâts bonus : +"
              << nouvelleArme.getBonusDegatsMin()
              << " à +"
              << nouvelleArme.getBonusDegatsMax()
              << std::endl;
    std::cout << "Critique bonus : +" << nouvelleArme.getBonusCritique() << std::endl;

    if (nouvelleArme.estIndestructible())
    {
        std::cout << "Durabilité : Indestructible" << std::endl;
    }
    else
    {
        std::cout << "Durabilité : "
                  << nouvelleArme.getDurabilite()
                  << "/"
                  << nouvelleArme.getDurabiliteMax()
                  << std::endl;
    }

    if (joueur.aUneArmeEquipee())
    {
        Arme armeActuelle = joueur.getArmeEquipee();

        std::cout << std::endl;
        std::cout << "Différence dégâts min : "
                  << nouvelleArme.getBonusDegatsMin() - armeActuelle.getBonusDegatsMin()
                  << std::endl;
        std::cout << "Différence dégâts max : "
                  << nouvelleArme.getBonusDegatsMax() - armeActuelle.getBonusDegatsMax()
                  << std::endl;
        std::cout << "Différence critique : "
                  << nouvelleArme.getBonusCritique() - armeActuelle.getBonusCritique()
                  << std::endl;
    }

    std::cout << "===========================================" << std::endl;
    std::cout << std::endl;
}

void MenuEquipement::afficherComparaisonArmure(const Joueur& joueur, const Armure& nouvelleArmure)
{
    std::cout << "========== COMPARAISON D'ARMURE ==========" << std::endl;

    if (joueur.aUneArmureEquipee())
    {
        Armure armureActuelle = joueur.getArmureEquipee();

        std::cout << "Armure actuelle : " << armureActuelle.getNom() << std::endl;
        std::cout << "Bonus PV max : +" << armureActuelle.getBonusPvMax() << std::endl;
        std::cout << "Réduction dégâts : " << armureActuelle.getReductionDegats() << std::endl;

        if (armureActuelle.estIndestructible())
        {
            std::cout << "Durabilité : Indestructible" << std::endl;
        }
        else
        {
            std::cout << "Durabilité : "
                      << armureActuelle.getDurabilite()
                      << "/"
                      << armureActuelle.getDurabiliteMax()
                      << std::endl;
        }

        std::cout << std::endl;
    }
    else
    {
        std::cout << "Armure actuelle : Aucune" << std::endl;
        std::cout << std::endl;
    }

    std::cout << "Nouvelle armure : " << nouvelleArmure.getNom() << std::endl;
    std::cout << "Bonus PV max : +" << nouvelleArmure.getBonusPvMax() << std::endl;
    std::cout << "Réduction dégâts : " << nouvelleArmure.getReductionDegats() << std::endl;

    if (nouvelleArmure.estIndestructible())
    {
        std::cout << "Durabilité : Indestructible" << std::endl;
    }
    else
    {
        std::cout << "Durabilité : "
                  << nouvelleArmure.getDurabilite()
                  << "/"
                  << nouvelleArmure.getDurabiliteMax()
                  << std::endl;
    }

    if (joueur.aUneArmureEquipee())
    {
        Armure armureActuelle = joueur.getArmureEquipee();

        std::cout << std::endl;
        std::cout << "Différence PV max : "
                  << nouvelleArmure.getBonusPvMax() - armureActuelle.getBonusPvMax()
                  << std::endl;
        std::cout << "Différence réduction : "
                  << nouvelleArmure.getReductionDegats() - armureActuelle.getReductionDegats()
                  << std::endl;
    }

    std::cout << "===========================================" << std::endl;
    std::cout << std::endl;
}

bool MenuEquipement::equiperArmeDepuisInventaire(Joueur& joueur)
{
    if (joueur.getInventaire().getNombreArmes() <= 0)
    {
        std::cout << joueur.getNom() << " n'a aucune arme à équiper." << std::endl;
        std::cout << std::endl;
        return false;
    }

    joueur.getInventaire().afficherListeArmes();

    std::cout << "Choisis l'arme à équiper." << std::endl;
    std::cout << "Entre son numéro, ou -1 pour annuler." << std::endl;
    std::cout << "> ";

    int choix = Console::demanderNombreEntre(
        -1,
        joueur.getInventaire().getNombreArmes() - 1,
        "Choix invalide. Entre un numéro d'arme valide, ou -1 pour annuler."
    );

    Console::clear();

    if (choix == -1)
    {
        std::cout << "Changement d'arme annulé." << std::endl;
        std::cout << std::endl;
        return false;
    }

    if (!joueur.getInventaire().possedeArme(choix))
    {
        std::cout << "Cette arme n'existe pas dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Arme nouvelleArme = joueur.getInventaire().getArme(choix);

    afficherComparaisonArme(joueur, nouvelleArme);

    std::cout << "Équiper cette arme ?" << std::endl;
    std::cout << "1 : Oui" << std::endl;
    std::cout << "0 : Non" << std::endl;
    std::cout << "> ";

    int confirmation = Console::demanderNombreEntre(
        0,
        1,
        "Choix invalide. Entre 0 ou 1."
    );

    Console::clear();

    if (confirmation == 0)
    {
        std::cout << "Changement d'arme annulé." << std::endl;
        std::cout << std::endl;
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

    joueur.getInventaire().afficherListeArmures();

    std::cout << "Choisis l'armure à équiper." << std::endl;
    std::cout << "Entre son numéro, ou -1 pour annuler." << std::endl;
    std::cout << "> ";

    int choix = Console::demanderNombreEntre(
        -1,
        joueur.getInventaire().getNombreArmures() - 1,
        "Choix invalide. Entre un numéro d'armure valide, ou -1 pour annuler."
    );

    Console::clear();

    if (choix == -1)
    {
        std::cout << "Changement d'armure annulé." << std::endl;
        std::cout << std::endl;
        return false;
    }

    if (!joueur.getInventaire().possedeArmure(choix))
    {
        std::cout << "Cette armure n'existe pas dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Armure nouvelleArmure = joueur.getInventaire().getArmure(choix);

    afficherComparaisonArmure(joueur, nouvelleArmure);

    std::cout << "Équiper cette armure ?" << std::endl;
    std::cout << "1 : Oui" << std::endl;
    std::cout << "0 : Non" << std::endl;
    std::cout << "> ";

    int confirmation = Console::demanderNombreEntre(
        0,
        1,
        "Choix invalide. Entre 0 ou 1."
    );

    Console::clear();

    if (confirmation == 0)
    {
        std::cout << "Changement d'armure annulé." << std::endl;
        std::cout << std::endl;
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