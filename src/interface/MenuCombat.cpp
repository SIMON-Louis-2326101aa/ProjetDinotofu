#include "interface/MenuCombat.hpp"

#include "core/Console.hpp"

#include <iostream>

void MenuCombat::afficherMenuTour(const Entite& entite)
{
    std::cout << "Tour de " << entite.getNom() << std::endl;
    std::cout << std::endl;

    std::cout << "Choisis une option parmi :" << std::endl;
    std::cout << "0 : Stats                 1 : Attaque" << std::endl;
    std::cout << "2 : Potion de soin        3 : Potion de dégâts" << std::endl;
    std::cout << "4 : Manuel de potions     5 : Passer son tour" << std::endl;
    std::cout << "6 : Inventaire            7 : Équipement" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void MenuCombat::afficherManuelPotions(int soinPotion, int bonusPotionDegats)
{
    std::cout << "========== MANUEL DES POTIONS ==========" << std::endl;
    std::cout << "Potion de soin :" << std::endl;
    std::cout << "- Rend " << soinPotion << " PV." << std::endl;
    std::cout << "- Elle ne peut pas dépasser les PV maximum." << std::endl;
    std::cout << std::endl;

    std::cout << "Potion de rage :" << std::endl;
    std::cout << "- Ajoute +" << bonusPotionDegats << " dégâts à la prochaine attaque." << std::endl;
    std::cout << "- L'attaque peut toujours être esquivée." << std::endl;
    std::cout << "- Si elle touche, l'arme perd de la durabilité normalement." << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
}

void MenuCombat::afficherPvApresAttaque(const Entite& defenseur)
{
    if (defenseur.statsVisibles())
    {
        std::cout << defenseur.getNom() << " possède maintenant "
                  << defenseur.getPv() << "/" << defenseur.getPvMax()
                  << " PV." << std::endl;
    }
    else
    {
        std::cout << "Les PV de " << defenseur.getNom()
                  << " restent impossibles à lire." << std::endl;
    }

    std::cout << std::endl;
}

void MenuCombat::afficherResultatCombat(const Entite& joueur1, const Entite& joueur2)
{
    Console::pauseSecondes(2);

    std::cout << std::endl;

    if (joueur1.estMort())
    {
        std::cout << joueur1.getNom() << " a été mis à terre..." << std::endl;
        std::cout << joueur2.getNom()
                  << " remporte donc la partie, ainsi que le respect de l'arène."
                  << std::endl;
    }
    else
    {
        std::cout << joueur2.getNom() << " a été mis à terre..." << std::endl;
        std::cout << joueur1.getNom()
                  << " remporte donc la partie, ainsi que le respect de l'arène."
                  << std::endl;
    }

    std::cout << std::endl;
}