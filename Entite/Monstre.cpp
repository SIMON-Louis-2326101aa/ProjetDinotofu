#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "Entite.cpp"
using namespace std;

class Monstre : public Entite {
public:
    Monstre(string nom, int pv, int atk) : Entite(nom, pv, atk) {}


};