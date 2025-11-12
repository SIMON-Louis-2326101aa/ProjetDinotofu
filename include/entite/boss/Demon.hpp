#ifndef BOSS_DEMON_HPP
#define BOSS_DEMON_HPP

#include "entite/monstre/Monstre.hpp"

class Demon : public Monstre {
public:
    Demon(); // valeurs par défaut “prédéfinies”
    Demon(const std::string& nom, int pv=140, int pvMax=140, int dmin=6, int dmax=14, int dcrit=22, int butin=60);

    // Effet spécial : saignement pendant N tours (exemple simple)
    // Retourne le nombre de PV drainés (pour feedback UI)
    int appliquerSaignement(Entite& cible, int tours, int parTour) const;
};

#endif
