#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <random>
#include <math.h>

#include "./Entite/Personnage.cpp"
#include "./Entite/Monstre/Monstre.cpp"
#include "./FonctionNotable/Affichage.cpp"
#include "./FonctionNotable/Action.cpp"

using namespace std;

int main()
{
    Personnage joueur1 = Personnage("Hazak", "Elf", "Combatant", 100, 100, 20, 25, 40);
    Personnage joueur2 = Personnage("Fail", "Fee", "Mage", 50, 50, 25, 35, 45);

    // Création de 2 objets de type Personnage : hazak et fail
    int poHFaible = 20;

    joueur2.attaquer(joueur1);              // fail attaque hazak
    joueur1.boirePotionDeVie(poHFaible); // hazak récupère 20 de vie en buvant une potion
    joueur2.attaquer(joueur1);              // fail attaque hazak
    joueur1.attaquer(joueur2);              // hazak contre-attaque...
    joueur2.attaquer(joueur1);

    queue<Monstre> reserveMonstres;
    reserveMonstres.push(Monstre("Troll", 40, 8));
    reserveMonstres.push(Monstre("Spectre", 35, 12));

    // Liste des monstres actuellement en combat
    std::vector<Monstre> monstresActifs = {
        Monstre("Gobelin", 30, 5),
        Monstre("Orc", 50, 10),
        Monstre("Dragon", 100, 20)};

    while (!monstresActifs.empty())
    {
        int action;
        do
        {
            std::cout << "\nActions disponibles :\n";
            std::cout << "1. Attaquer un monstre\n";
            std::cout << "2. Afficher mes stats\n";
            std::cout << "Choisissez une action : ";
            std::cin >> action;

            if (action == 2)
            {
                joueur.afficherStats();
            }
        } while (action != 1);

        afficherMonstres(monstresActifs);
        int indexCible = choisirCible(monstresActifs);
        joueur.attaquer(monstresActifs[indexCible]);

        if (monstresActifs[indexCible].estMort())
        {
            std::cout << monstresActifs[indexCible].getNom() << " est vaincu !" << std::endl;
            monstresActifs.erase(monstresActifs.begin() + indexCible); // Retirer le monstre mort

            // Ajouter un nouveau monstre s'il en reste
            if (!reserveMonstres.empty())
            {
                monstresActifs.push_back(reserveMonstres.front());
                reserveMonstres.pop();
                std::cout << monstresActifs.back().getNom() << " entre en combat !" << std::endl;
            }
        }

        // Tour des monstres encore vivants
        for (auto &monstre : monstresActifs)
        {
            monstre.attaquer(joueur);
            if (joueur.estMort())
            {
                std::cout << "Le joueur est mort ! Game Over." << std::endl;
                return 0;
            }
        }
    }

    std::cout << "Victoire ! Tous les monstres sont vaincus." << std::endl;
    return 0;
}