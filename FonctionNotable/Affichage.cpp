#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <random>
#include <math.h>

#include "../Entite/Entite.cpp"
#include "../Entite/Personnage.cpp"
#include "../Entite/Monstre/Monstre.cpp"

using namespace std;

class Affichage
{
    void afficherMonstres(const vector<Monstre> &monstres)
    {
        cout << "\nMonstres en combat :\n";
        for (size_t i = 0; i < monstres.size(); i++)
        {
            cout << i + 1 << ". " << monstres[i].getNom() << " (PV: " << monstres[i].getPV() << "/" << monstres[i].getPvMax() << ")\n";
        }
    }

    void afficherJoueur(vector<Personnage> &joueur)
    {
        for (size_t i = 0; i < joueur.size(); i++)
        {
        cout << "\nJoueur : " << joueur[i].getNom() << endl;
        cout << " (PV: " << joueur[i].getPV() << "/" << joueur.getPvMax() << ")\n";
        }
    }

    void afficherStats(Entite)
    {
        cout << "\n--- Stats de Entite ---\n";
        cout << "Nom: " << Entite.getNom() << "\n";
        cout << "PV: " << Entite.getPV() << "\n";
        cout << "Attaque: " << joueur.getDgtMin() << "\n";
        cout << "-----------------------\n";
    }
}