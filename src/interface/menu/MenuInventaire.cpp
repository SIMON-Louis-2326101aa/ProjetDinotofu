#include "interface/menu/MenuInventaire.hpp"

#include "core/Console.hpp"

#include "interface/menu/inventaire/InventaireAffichage.hpp"
#include "interface/menu/inventaire/InventaireSelection.hpp"

bool MenuInventaire::ouvrir(Joueur& joueur)
{
    while (true)
    {
        InventaireAffichage::afficherMenuPrincipal();

        int choixMenu = Console::demanderNombreEntre(
            0,
            5,
            "Choix invalide. Entre un chiffre entre 0 et 5."
        );

        Console::clear();

        if (choixMenu == 0)
        {
            return false;
        }

        if (choixMenu == 1)
        {
            InventaireAffichage::afficherToutSimple(joueur);
            continue;
        }

        if (choixMenu == 2)
        {
            InventaireSelection::ouvrirArmes(joueur);
            continue;
        }

        if (choixMenu == 3)
        {
            InventaireSelection::ouvrirArmures(joueur);
            continue;
        }

        if (choixMenu == 4)
        {
            bool tourConsomme = InventaireSelection::ouvrirConsommables(joueur);

            if (tourConsomme)
            {
                return true;
            }

            continue;
        }

        if (choixMenu == 5)
        {
            InventaireSelection::ouvrirMateriaux(joueur);
            continue;
        }
    }
}