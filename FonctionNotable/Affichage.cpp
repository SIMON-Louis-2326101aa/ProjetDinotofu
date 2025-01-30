#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <random>
#include <math.h>
#include "../Entite/Personnage.cpp"
#include "../Entite/Monstre.cpp"
using namespace std;
class Affichage
{
    void afficherMonstres(const std::vector<Monstre> &monstres)
    {
        cout << "\nMonstres en combat :\n";
        for (size_t i = 0; i < monstres.size(); i++)
        {
            cout << i + 1 << ". " << monstres[i].getNom() << " (PV: " << monstres[i].getPV() << "/" << monstres[i].getPvMax() << ")\n";
        }
    }

    void afficherJoueur(const Personnage joueur)
    {
        cout << "\nJoueur : " << joueur.getNom() << endl;
        cout << " (PV: " << joueur.getPV() << "/" << joueur.getPvMax() << ")\n";
    }

    void afficherStats()
    {
        std::cout << "\n--- Stats du Joueur ---\n";
        std::cout << "Nom: " << getNom() << "\n";
        std::cout << "PV: " << getPV() << "\n";
        std::cout << "Attaque: " << getAttaque() << "\n";
        std::cout << "-----------------------\n";
    }
}