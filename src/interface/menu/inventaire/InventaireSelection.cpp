#include "interface/menu/inventaire/InventaireSelection.hpp"

#include "core/Console.hpp"

#include "interface/menu/inventaire/InventaireAffichage.hpp"
#include "interface/menu/inventaire/InventaireUtils.hpp"

#include "objet/Inventaire.hpp"
#include "objet/arme/Arme.hpp"
#include "objet/armure/Armure.hpp"
#include "objet/consommable/Consommable.hpp"
#include "objet/consommable/TypeConsommable.hpp"

#include <iostream>
#include <vector>

bool InventaireSelection::ouvrirArmes(Joueur& joueur)
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

    Arme arme = joueur.getInventaire().getArme(index);
    InventaireAffichage::afficherArmeSelectionnee(arme);

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
        }
        else
        {
            std::cout << "Impossible d'équiper cette arme." << std::endl;
            std::cout << std::endl;
        }
    }

    return false;
}

bool InventaireSelection::ouvrirArmures(Joueur& joueur)
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

    Armure armure = joueur.getInventaire().getArmure(index);
    InventaireAffichage::afficherArmureSelectionnee(armure);

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
        }
        else
        {
            std::cout << "Impossible d'équiper cette armure." << std::endl;
            std::cout << std::endl;
        }
    }

    return false;
}

bool InventaireSelection::ouvrirConsommables(Joueur& joueur)
{
    if (joueur.getInventaire().getNombreConsommables() <= 0)
    {
        std::cout << "Tu n'as aucun consommable dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    std::vector<GroupeConsommable> groupes = InventaireUtils::grouperConsommables(joueur);

    std::cout << "============ CONSOMMABLES ============" << std::endl;

    for (int i = 0; i < static_cast<int>(groupes.size()); ++i)
    {
        const GroupeConsommable& groupe = groupes[i];

        std::cout << i + 1
                  << " : "
                  << groupe.nom
                  << " x"
                  << groupe.quantite
                  << " | "
                  << InventaireUtils::typeConsommableVersTexte(groupe.type)
                  << " | Puissance : "
                  << groupe.puissance
                  << std::endl;
    }

    std::cout << "=======================================" << std::endl;
    std::cout << "Sélectionne un consommable, ou entre 0 pour revenir." << std::endl;
    std::cout << "> ";

    int choix = Console::demanderNombreEntre(
        0,
        static_cast<int>(groupes.size()),
        "Choix invalide. Sélectionne un consommable affiché, ou 0 pour revenir."
    );

    Console::clear();

    if (choix == 0)
    {
        return false;
    }

    int index = groupes[choix - 1].premierIndex;

    if (!joueur.getInventaire().possedeConsommable(index))
    {
        std::cout << "Ce consommable n'existe plus dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Consommable consommable = joueur.getInventaire().getConsommable(index);

    InventaireAffichage::afficherConsommableSelectionne(consommable);

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
            std::cout << "Ce consommable demande une cible ou un effet spécial." << std::endl;
            std::cout << "Utilise plutôt l'option Potions du menu de combat." << std::endl;
            std::cout << std::endl;
            return false;
        }

        joueur.soigner(consommable.getPuissance());
        joueur.getInventaire().retirerConsommable(index);

        std::cout << joueur.getNom() << " utilise : " << consommable.getNom() << "." << std::endl;
        std::cout << "Ses blessures se referment, et il récupère "
                  << consommable.getPuissance()
                  << " PV."
                  << std::endl;
        std::cout << joueur.getNom() << " possède maintenant "
                  << joueur.getPv()
                  << "/"
                  << joueur.getPvMax()
                  << " PV."
                  << std::endl;
        std::cout << std::endl;

        return true;
    }

    return false;
}

bool InventaireSelection::ouvrirMateriaux(Joueur& joueur)
{
    (void)joueur;

    InventaireAffichage::afficherMateriauxIndisponibles();

    return false;
}