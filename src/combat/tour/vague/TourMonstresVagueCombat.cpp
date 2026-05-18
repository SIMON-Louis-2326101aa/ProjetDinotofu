#include "combat/tour/vague/TourMonstresVagueCombat.hpp"

#include "combat/GestionnaireTours.hpp"
#include "combat/systeme/SystemeFuite.hpp"

#include "core/Console.hpp"

#include "entite/Monstre.hpp"

#include <iostream>

void TourMonstresVagueCombat::jouerToursMonstres(
    Joueur& joueur,
    FileEnnemisCombat& vague,
    Random& random
)
{
    int i = 0;

    while (i < vague.getNombreEnnemisActifs())
    {
        if (joueur.estMort())
        {
            return;
        }

        if (!vague.indexActifValide(i))
        {
            ++i;
            continue;
        }

        Monstre& monstre = vague.getEnnemiActif(i);

        if (monstre.estMort())
        {
            ++i;
            continue;
        }

        if (SystemeFuite::monstreTenteFuite(monstre, random))
        {
            vague.retirerEnnemiActif(i);
            continue;
        }

        std::cout << "Tour de " << monstre.getNom() << std::endl;
        std::cout << std::endl;

        Console::pauseSecondes(1);

        GestionnaireTours::executerAttaque(
            monstre,
            joueur,
            random
        );

        Console::pauseSecondes(1);

        ++i;
    }

    vague.retirerMortsEtRemplacer();
}