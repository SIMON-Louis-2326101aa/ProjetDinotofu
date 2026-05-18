#include "interface/menu/MenuPotionsCombat.hpp"

#include "core/Console.hpp"

#include "interface/menu/MenuCombat.hpp"
#include "interface/menu/potions/PotionsCombatAffichage.hpp"
#include "interface/menu/potions/PotionsCombatUtilisation.hpp"
#include "interface/menu/potions/PotionsCombatUtils.hpp"

#include "objet/Inventaire.hpp"
#include "objet/consommable/Consommable.hpp"

#include <iostream>

bool MenuPotionsCombat::ouvrirSoinRapide(Joueur& joueur)
{
    std::vector<int> indices = PotionsCombatUtils::recupererIndicesPotions(
        joueur,
        TypeConsommable::Soin
    );

    if (indices.empty())
    {
        std::cout << "Tu n'as aucune potion de soin disponible." << std::endl;
        std::cout << std::endl;
        return false;
    }

    while (true)
    {
        PotionsCombatAffichage::afficherSoinRapide(joueur, indices);

        int choix = Console::demanderNombreEntre(
            0,
            static_cast<int>(indices.size()),
            "Choix invalide. Sélectionne une potion affichée."
        );

        Console::clear();

        if (choix == 0)
        {
            return false;
        }

        int indexConsommable = indices[choix - 1];

        if (!joueur.getInventaire().possedeConsommable(indexConsommable))
        {
            std::cout << "Cette potion n'existe plus dans l'inventaire." << std::endl;
            std::cout << std::endl;
            return false;
        }

        Consommable potion = joueur.getInventaire().getConsommable(indexConsommable);

        PotionsCombatAffichage::afficherPotionSoinSelectionnee(potion);

        int action = Console::demanderNombreEntre(
            0,
            2,
            "Choix invalide. Entre 0, 1 ou 2."
        );

        Console::clear();

        if (action == 0)
        {
            return false;
        }

        if (action == 1)
        {
            potion.afficher();
            return false;
        }

        if (action == 2)
        {
            return PotionsCombatUtilisation::utiliserPotionSoin(
                joueur,
                indexConsommable,
                potion
            );
        }
    }
}

bool MenuPotionsCombat::ouvrirContreCibleUnique(
    Joueur& joueur,
    Entite& cible,
    Random& random,
    int bonusPotionDegats
)
{
    while (true)
    {
        PotionsCombatAffichage::afficherMenuPrincipal();

        int choix = Console::demanderNombreEntre(
            0,
            6,
            "Choix invalide. Entre un chiffre entre 0 et 6."
        );

        Console::clear();

        if (choix == 0)
        {
            return false;
        }

        if (choix == 1)
        {
            PotionsCombatAffichage::afficherPotions(joueur);
            return false;
        }

        if (choix == 2)
        {
            return ouvrirCategorie(
                joueur,
                TypeConsommable::Soin,
                &cible,
                nullptr,
                random,
                bonusPotionDegats
            );
        }

        if (choix == 3)
        {
            MenuCombat::afficherOptionNonDisponible();
            return false;
        }

        if (choix == 4)
        {
            return ouvrirCategorie(
                joueur,
                TypeConsommable::Degats,
                &cible,
                nullptr,
                random,
                bonusPotionDegats
            );
        }

        if (choix == 5)
        {
            return ouvrirCategorie(
                joueur,
                TypeConsommable::Buff,
                &cible,
                nullptr,
                random,
                bonusPotionDegats
            );
        }

        if (choix == 6)
        {
            return ouvrirCategorie(
                joueur,
                TypeConsommable::Debuff,
                &cible,
                nullptr,
                random,
                bonusPotionDegats
            );
        }
    }
}

bool MenuPotionsCombat::ouvrirContreVague(
    Joueur& joueur,
    FileEnnemisCombat& vague,
    Random& random,
    int bonusPotionDegats
)
{
    while (true)
    {
        PotionsCombatAffichage::afficherMenuPrincipal();

        int choix = Console::demanderNombreEntre(
            0,
            6,
            "Choix invalide. Entre un chiffre entre 0 et 6."
        );

        Console::clear();

        if (choix == 0)
        {
            return false;
        }

        if (choix == 1)
        {
            PotionsCombatAffichage::afficherPotions(joueur);
            return false;
        }

        if (choix == 2)
        {
            return ouvrirCategorie(
                joueur,
                TypeConsommable::Soin,
                nullptr,
                &vague,
                random,
                bonusPotionDegats
            );
        }

        if (choix == 3)
        {
            MenuCombat::afficherOptionNonDisponible();
            return false;
        }

        if (choix == 4)
        {
            return ouvrirCategorie(
                joueur,
                TypeConsommable::Degats,
                nullptr,
                &vague,
                random,
                bonusPotionDegats
            );
        }

        if (choix == 5)
        {
            return ouvrirCategorie(
                joueur,
                TypeConsommable::Buff,
                nullptr,
                &vague,
                random,
                bonusPotionDegats
            );
        }

        if (choix == 6)
        {
            return ouvrirCategorie(
                joueur,
                TypeConsommable::Debuff,
                nullptr,
                &vague,
                random,
                bonusPotionDegats
            );
        }
    }
}

bool MenuPotionsCombat::ouvrirCategorie(
    Joueur& joueur,
    TypeConsommable type,
    Entite* cible,
    FileEnnemisCombat* vague,
    Random& random,
    int bonusPotionDegats
)
{
    std::vector<int> indices = PotionsCombatUtils::recupererIndicesPotions(
        joueur,
        type
    );

    if (indices.empty())
    {
        std::cout << "Tu n'as aucune potion de type "
                  << PotionsCombatUtils::typeVersTexte(type)
                  << "."
                  << std::endl;
        std::cout << std::endl;

        return false;
    }

    return ouvrirSelectionPotion(
        joueur,
        indices,
        type,
        cible,
        vague,
        random,
        bonusPotionDegats
    );
}

bool MenuPotionsCombat::ouvrirSelectionPotion(
    Joueur& joueur,
    const std::vector<int>& indices,
    TypeConsommable type,
    Entite* cible,
    FileEnnemisCombat* vague,
    Random& random,
    int bonusPotionDegats
)
{
    if (indices.empty())
    {
        return false;
    }

    while (true)
    {
        PotionsCombatAffichage::afficherPotionsFiltrees(joueur, indices);

        int choix = Console::demanderNombreEntre(
            0,
            static_cast<int>(indices.size()),
            "Choix invalide. Sélectionne une potion affichée."
        );

        Console::clear();

        if (choix == 0)
        {
            return false;
        }

        int indexConsommable = indices[choix - 1];

        if (!joueur.getInventaire().possedeConsommable(indexConsommable))
        {
            std::cout << "Cette potion n'existe plus dans l'inventaire." << std::endl;
            std::cout << std::endl;
            return false;
        }

        Consommable potion = joueur.getInventaire().getConsommable(indexConsommable);

        PotionsCombatAffichage::afficherPotionSelectionnee(potion);

        int action = Console::demanderNombreEntre(
            0,
            2,
            "Choix invalide. Entre 0, 1 ou 2."
        );

        Console::clear();

        if (action == 0)
        {
            return false;
        }

        if (action == 1)
        {
            potion.afficher();
            return false;
        }

        if (action == 2)
        {
            return PotionsCombatUtilisation::utiliserPotionSelectionnee(
                joueur,
                indexConsommable,
                type,
                cible,
                vague,
                random,
                bonusPotionDegats
            );
        }
    }
}