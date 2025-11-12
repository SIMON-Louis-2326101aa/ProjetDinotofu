#ifndef MONSTRE_DEFS_HPP
#define MONSTRE_DEFS_HPP

#include <string>
#include <vector>
#include "entite/monstre/Monstre.hpp"

struct MonstreDef {
    std::string nom, type;
    int pv, pvMax, dmin, dmax, dcrit, butin;
};

namespace MonstresCatalogue {
    inline const std::vector<MonstreDef> Defs = {
        {"Lutin farceur", "Fée", 70, 70, 3, 8, 15, 12},
        {"Garde de pierre", "Protecteur", 120, 120, 4, 10, 18, 25},
        {"Ange déchu", "Ange", 100, 100, 5, 11, 19, 40},
    };

    inline Monstre creer(const MonstreDef& d) {
        return Monstre(d.nom, d.type, d.pv, d.pvMax, d.dmin, d.dmax, d.dcrit, d.butin);
    }
}

#endif
