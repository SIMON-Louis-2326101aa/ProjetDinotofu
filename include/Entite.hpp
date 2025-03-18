#include <iostream>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

class Entite
{
private:
        string nom;
        string race;
        string classe;
        int pv;
        int pvMax;
        int dgtMin;
        int dgtMax;
        int dgtCrit;

public:
        // Getters
        inline string getNom() const { return nom; }
        inline string getRace() const { return race; }
        string getClasse() const { return classe; }
        int getPV() const { return pv; }
        int getPvMax() const { return pvMax; }
        int getDgtMin() const { return dgtMin; }
        int getDgtMax() const { return dgtMax; }
        int getDgtCrit() const { return dgtCrit; }
        

        // Setters
        string setNom(string newNom) { nom = newNom; }
        string setRace(string newRace) { race = newRace; }
        string setClasse(string newClasse) { classe = newClasse; }
        void setPV(int newPV) { pv = newPV; }
        int setPvMax(int newPvMax) { pvMax = newPvMax; }
        int setDgtMin(int newDgtMin) { dgtMin = newDgtMin; }
        int setDgtMax(int newDgtMax) { dgtMax = newDgtMax; }
        int setDgtCrit(int newDgtCrit) { dgtCrit = newDgtCrit; }
        Entite(string nom, string race, string classe, int pv, int pvMax, int dgtMin, int dgtMax, int dgtCrit) {}
};