#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <random>
#include <math.h>

#include "../Entite/Personnage.cpp"
#include "../Entite/Monstre/Monstre.cpp"

using namespace std;

class ActionPerso : public Personnage
{
    void recevoirDegats(int pv, int dgt)
    {
        pv = pv - dgt;
        estVivant(pv);
    }

    void attaquer(Personnage &cible)
    {
    }

    void boirePotionDeVie(int poH)
    {
    }

    bool estVivant(int pv)
    {
        if (pv <= 0){
            pv = 0;
            return pv;
        }
    }

    int choisirCible(const std::vector<Monstre> &monstres)
    {
        int choix;
        do
        {
            std::cout << "Choisissez un monstre à attaquer (1-" << monstres.size() << "): ";
            std::cin >> choix;
        } while (choix < 1 || choix > static_cast<int>(monstres.size()));
        return choix - 1;
    }
};