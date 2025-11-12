#include "entite/boss/Demon.hpp"

Demon::Demon()
: Demon("Démon ancien") {}

Demon::Demon(const std::string& nom, int pv, int pvMax, int dmin, int dmax, int dcrit, int butin)
: Monstre(nom, "Démon", pv, pvMax, dmin, dmax, dcrit, butin) {}

int Demon::appliquerSaignement(Entite& cible, int tours, int parTour) const {
    int total = 0;
    for (int i=0;i<tours;++i) {
        int avant = cible.getPV();
        cible.prendreDegats(parTour);
        total += (avant - cible.getPV());
        if (!cible.estVivant()) break;
    }
    return total;
}
