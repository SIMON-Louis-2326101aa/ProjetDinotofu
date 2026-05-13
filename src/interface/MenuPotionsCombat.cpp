#include "interface/MenuPotionsCombat.hpp"

#include "core/Console.hpp"

#include "combat/GestionnaireTours.hpp"

#include "interface/MenuCibleCombat.hpp"
#include "interface/MenuCombat.hpp"

#include "objet/Inventaire.hpp"
#include "objet/consommable/Consommable.hpp"

#include <iostream>

bool MenuPotionsCombat::ouvrirSoinRapide(Joueur& joueur)
{
    std::vector<int> indices = recupererIndicesPotions(
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
        std::cout << "========== POTION DE SOIN RAPIDE ==========" << std::endl;

        for (int i = 0; i < static_cast<int>(indices.size()); ++i)
        {
            int indexInventaire = indices[i];
            Consommable potion = joueur.getInventaire().getConsommable(indexInventaire);

            std::cout << i + 1
                      << " : "
                      << potion.getNom()
                      << " | Soin : "
                      << potion.getPuissance()
                      << " PV"
                      << std::endl;
        }

        std::cout << "===========================================" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

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

        std::cout << "========== POTION SÉLECTIONNÉE ==========" << std::endl;
        std::cout << "Potion : " << potion.getNom() << std::endl;
        std::cout << "Description : " << potion.getDescription() << std::endl;
        std::cout << "Soin : " << potion.getPuissance() << " PV" << std::endl;
        std::cout << "=========================================" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Inspecter" << std::endl;
        std::cout << "2 : Utiliser" << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

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
            joueur.soigner(potion.getPuissance());
            joueur.getInventaire().retirerConsommable(indexConsommable);

            std::cout << joueur.getNom()
                      << " boit "
                      << potion.getNom()
                      << " et récupère "
                      << potion.getPuissance()
                      << " PV."
                      << std::endl;
            std::cout << std::endl;

            return true;
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
    bool quitter = false;

    while (!quitter)
    {
        afficherMenuPrincipal();

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
            afficherPotions(joueur);
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

    return false;
}

bool MenuPotionsCombat::ouvrirContreVague(
    Joueur& joueur,
    FileEnnemisCombat& vague,
    Random& random,
    int bonusPotionDegats
)
{
    bool quitter = false;

    while (!quitter)
    {
        afficherMenuPrincipal();

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
            afficherPotions(joueur);
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

    return false;
}

void MenuPotionsCombat::afficherMenuPrincipal()
{
    std::cout << "========== POTIONS ==========" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Voir les potions" << std::endl;
    std::cout << "2 : Utiliser une potion curative" << std::endl;
    std::cout << "3 : Utiliser une potion défensive" << std::endl;
    std::cout << "4 : Utiliser une potion offensive" << std::endl;
    std::cout << "5 : Utiliser une potion de buff" << std::endl;
    std::cout << "6 : Utiliser une potion de debuff" << std::endl;
    std::cout << "============================" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
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
    std::vector<int> indices = recupererIndicesPotions(joueur, type);

    if (indices.empty())
    {
        std::cout << "Tu n'as aucune potion de type "
                  << typeVersTexte(type)
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

std::vector<int> MenuPotionsCombat::recupererIndicesPotions(
    const Joueur& joueur,
    TypeConsommable type
)
{
    std::vector<int> indices;

    const std::vector<Consommable>& consommables =
        joueur.getInventaire().getConsommables();

    for (int i = 0; i < static_cast<int>(consommables.size()); ++i)
    {
        if (consommables[i].getType() == type)
        {
            indices.push_back(i);
        }
    }

    return indices;
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

    bool quitter = false;

    while (!quitter)
    {
        afficherPotionsFiltrees(joueur, indices);

        std::cout << "Choisis une potion." << std::endl;
        std::cout << "Entre son numéro dans la liste, ou 0 pour revenir." << std::endl;
        std::cout << "> ";

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

        std::cout << "========== POTION SÉLECTIONNÉE ==========" << std::endl;
        std::cout << "Potion : " << potion.getNom() << std::endl;
        std::cout << "Type : " << typeVersTexte(potion.getType()) << std::endl;
        std::cout << "Puissance : " << potion.getPuissance() << std::endl;
        std::cout << "=========================================" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Inspecter" << std::endl;
        std::cout << "2 : Utiliser" << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

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
            return utiliserPotionSelectionnee(
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

    return false;
}

bool MenuPotionsCombat::utiliserPotionSelectionnee(
    Joueur& joueur,
    int indexConsommable,
    TypeConsommable type,
    Entite* cible,
    FileEnnemisCombat* vague,
    Random& random,
    int bonusPotionDegats
)
{
    if (!joueur.getInventaire().possedeConsommable(indexConsommable))
    {
        std::cout << "Cette potion n'est plus disponible." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Consommable potion = joueur.getInventaire().getConsommable(indexConsommable);

    if (type == TypeConsommable::Soin)
    {
        joueur.soigner(potion.getPuissance());
        joueur.getInventaire().retirerConsommable(indexConsommable);

        std::cout << joueur.getNom()
                  << " boit "
                  << potion.getNom()
                  << " et récupère "
                  << potion.getPuissance()
                  << " PV."
                  << std::endl;
        std::cout << std::endl;

        return true;
    }

    if (type == TypeConsommable::Degats)
    {
        if (vague != nullptr)
        {
            return MenuCibleCombat::ouvrirPourPotionDegats(
                joueur,
                *vague,
                random,
                bonusPotionDegats
            );
        }

        if (cible != nullptr)
        {
            joueur.getInventaire().retirerConsommable(indexConsommable);

            std::cout << joueur.getNom()
                      << " utilise "
                      << potion.getNom()
                      << "."
                      << std::endl;
            std::cout << std::endl;

            bool esquive = false;
            bool critique = false;

            int degats = joueur.attaquer(
                random,
                esquive,
                critique,
                bonusPotionDegats + potion.getPuissance()
            );

            if (esquive)
            {
                std::cout << cible->getNom()
                          << " esquive l'attaque renforcée."
                          << std::endl;
                std::cout << std::endl;
                return true;
            }

            if (critique)
            {
                std::cout << "Coup critique sous potion !" << std::endl;
            }

            cible->recevoirDegats(degats);

            std::cout << joueur.getNom()
                      << " inflige "
                      << degats
                      << " dégâts à "
                      << cible->getNom()
                      << "."
                      << std::endl;
            std::cout << std::endl;

            return true;
        }

        std::cout << "Aucune cible offensive disponible." << std::endl;
        std::cout << std::endl;
        return false;
    }

    if (type == TypeConsommable::Buff || type == TypeConsommable::Debuff)
    {
        std::cout << "[cette option n'est pas encore accessible pour ce mode]" << std::endl;
        std::cout << "La potion existe, mais son effet n'est pas encore codé." << std::endl;
        std::cout << std::endl;
        return false;
    }

    std::cout << "[cette option n'est pas encore accessible pour ce mode]" << std::endl;
    std::cout << std::endl;

    return false;
}

void MenuPotionsCombat::afficherPotions(const Joueur& joueur)
{
    const std::vector<Consommable>& consommables =
        joueur.getInventaire().getConsommables();

    std::cout << "========== POTIONS DISPONIBLES ==========" << std::endl;

    if (consommables.empty())
    {
        std::cout << "Aucune potion dans l'inventaire." << std::endl;
        std::cout << "=========================================" << std::endl;
        std::cout << std::endl;
        return;
    }

    for (int i = 0; i < static_cast<int>(consommables.size()); ++i)
    {
        const Consommable& potion = consommables[i];

        std::cout << "[" << i << "] "
                  << potion.getNom()
                  << " | "
                  << typeVersTexte(potion.getType())
                  << " | Puissance : "
                  << potion.getPuissance()
                  << std::endl;
    }

    std::cout << "=========================================" << std::endl;
    std::cout << std::endl;
}

void MenuPotionsCombat::afficherPotionsFiltrees(
    const Joueur& joueur,
    const std::vector<int>& indices
)
{
    std::cout << "========== LISTE DES POTIONS ==========" << std::endl;

    for (int i = 0; i < static_cast<int>(indices.size()); ++i)
    {
        int indexInventaire = indices[i];
        Consommable potion = joueur.getInventaire().getConsommable(indexInventaire);

        std::cout << i + 1
                  << " : "
                  << potion.getNom()
                  << " | Puissance : "
                  << potion.getPuissance()
                  << std::endl;
    }

    std::cout << "=======================================" << std::endl;
    std::cout << std::endl;
}

std::string MenuPotionsCombat::typeVersTexte(TypeConsommable type)
{
    switch (type)
    {
        case TypeConsommable::Soin:
            return "Curative";

        case TypeConsommable::Degats:
            return "Offensive";

        case TypeConsommable::Buff:
            return "Buff";

        case TypeConsommable::Debuff:
            return "Debuff";

        case TypeConsommable::Special:
            return "Spéciale";

        default:
            return "Inconnue";
    }
}