#include "interface/MenuCibleCombat.hpp"

#include "combat/GestionnaireTours.hpp"
#include "combat/SystemeObservation.hpp"

#include "core/Console.hpp"

#include <iostream>

bool MenuCibleCombat::ouvrirPourAttaque(
    Joueur& joueur,
    FileEnnemisCombat& vague,
    Random& random
)
{
    return ouvrirMenuCible(
        joueur,
        vague,
        random,
        false,
        0
    );
}

bool MenuCibleCombat::ouvrirPourPotionDegats(
    Joueur& joueur,
    FileEnnemisCombat& vague,
    Random& random,
    int bonusPotionDegats
)
{
    return ouvrirMenuCible(
        joueur,
        vague,
        random,
        true,
        bonusPotionDegats
    );
}

int MenuCibleCombat::choisirCible(const FileEnnemisCombat& vague)
{
    vague.afficherEnnemisActifs();

    std::cout << "Choisis une cible." << std::endl;
    std::cout << "Entre le numéro de l'ennemi à sélectionner." << std::endl;
    std::cout << "> ";

    return Console::demanderNombreEntre(
        0,
        vague.getNombreEnnemisActifs() - 1,
        "Cible invalide. Choisis un ennemi actif."
    );
}

bool MenuCibleCombat::ouvrirMenuCible(
    Joueur& joueur,
    FileEnnemisCombat& vague,
    Random& random,
    bool attaqueAvecPotionDegats,
    int bonusPotionDegats
)
{
    while (vague.aEncoreDesEnnemis())
    {
        int indexCible = choisirCible(vague);

        Console::clear();

        if (!vague.indexActifValide(indexCible))
        {
            std::cout << "Cette cible n'est plus disponible." << std::endl;
            std::cout << std::endl;
            return false;
        }

        Monstre& cible = vague.getEnnemiActif(indexCible);

        bool resterSurCetteCible = true;

        while (resterSurCetteCible && !cible.estMort())
        {
            std::cout << "========== CIBLE SÉLECTIONNÉE ==========" << std::endl;
            std::cout << "Cible : " << cible.getNom() << std::endl;
            std::cout << "Race : " << cible.getRaceTexte() << std::endl;

            if (cible.estInvocation())
            {
                std::cout << "Statut : Invocation" << std::endl;
            }
            else if (cible.estElite())
            {
                std::cout << "Statut : Élite" << std::endl;
            }
            else
            {
                std::cout << "Statut : Ennemi standard" << std::endl;
            }

            std::cout << "========================================" << std::endl;
            std::cout << "0 : Retour au menu principal" << std::endl;

            if (attaqueAvecPotionDegats)
            {
                std::cout << "1 : Utiliser la potion de rage sur cette cible" << std::endl;
            }
            else
            {
                std::cout << "1 : Attaquer cette cible" << std::endl;
            }

            std::cout << "2 : Inspecter cette cible" << std::endl;
            std::cout << "3 : Choisir une autre cible" << std::endl;
            std::cout << std::endl;
            std::cout << "> ";

            int choix = Console::demanderNombreEntre(
                0,
                3,
                "Choix invalide. Entre un chiffre entre 0 et 3."
            );

            Console::clear();

            if (choix == 0)
            {
                return false;
            }

            if (choix == 1)
            {
                if (attaqueAvecPotionDegats)
                {
                    bool tourConsomme = GestionnaireTours::executerPotionDegats(
                        joueur,
                        cible,
                        random,
                        bonusPotionDegats
                    );

                    vague.retirerMortsEtRemplacer();

                    return tourConsomme;
                }

                GestionnaireTours::executerAttaque(joueur, cible, random);

                vague.retirerMortsEtRemplacer();

                return true;
            }

            if (choix == 2)
            {
                SystemeObservation::afficherStatsTerminal(cible);
            }

            if (choix == 3)
            {
                resterSurCetteCible = false;
            }
        }
    }

    return false;
}