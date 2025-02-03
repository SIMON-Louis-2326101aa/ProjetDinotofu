#include <iostream>
#include <stdlib.h>
#include <unistd.h>

#include "Entite.cpp"

using namespace std;

class Monstre : public Entite {
public:
    Monstre(string nom, string race, string classe, int pv, int pvMax, int dgtMin, int dgtMax, int dgtCrit) : Entite(nom, race, classe, pv, pvMax, dgtMin, dgtMax, dgtCrit) {}


};