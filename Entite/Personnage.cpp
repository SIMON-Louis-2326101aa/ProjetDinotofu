#include <iostream>
#include <stdlib.h>
#include <unistd.h>

#include "Entite.cpp"

using namespace std;

class Personnage : Entite
{
    private :
        int poV;
        int poD;   

    public :
        Personnage(string nom, string race, string classe, int pv, int pvMax, int dgtMin, int dgtMax, int dgtCrit) : Entite(nom, race, classe, pv, pvMax, dgtMin, dgtMax, dgtCrit) {}
};

// Personnage(nom, class, pv, pvMax, dgtMin, dgtMax, dgtCrit, poV, poD)
//     Monstre(nom, race, class, pv, pvMax, dgtMin, dgtMax, dgtCrit, poV, poD)
//         Boss(nom, race, class, pv, pvMax, dgtMin, dgtMax, dgtCrit, poV, poD, ulti, ultiMax, delai, delaiMax)

//             chevalier(nom, "Chevalier", 200, 200, 5, 20, 30, 4, 3)
//                 assassin(nom, "Assassin", 150, 150, 10, 30, 45, 5, 2)
//                     tank(nom, "Tank", 400, 400, 5, 12, 20, 2, 5)

//                         bAnge("Fitoria", "Ange", 800, 800, 3, 10, 15, 3, 5, 3, 3, 14, 14)
//                             bDemon("Zelef", "Demon", 600, 600, 5, 15, 22, 5, 4, 4, 4, 10, 10)
//                                 bProtecteur("Atlas", "Protecteur Universel Déchu", 1200, 1200, 2, 5, 8, 2, 5, 3, 3, 12, 12)