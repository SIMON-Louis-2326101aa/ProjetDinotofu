#ifndef MONSTRE_HPP
#define MONSTRE_HPP

#include <string>
#include "entite/Entite.hpp"

class Monstre : public Entite {
private:
    std::string type; // "Démon", "Ange", "Protecteur", etc.
    int butinOr;      // or donné au joueur quand il meurt

public:
    Monstre();
    Monstre(const std::string& nom, const std::string& type,
            int pv, int pvMax, int dgtMin, int dgtMax, int dgtCrit,
            int butinOr = 0);
    virtual ~Monstre() = default;

    const std::string& getType() const;
    int getButinOr() const;
    void setType(const std::string& t);
    void setButinOr(int v);

    // Helpers d’ambiance/affichage (implémentés dans .cpp)
    void crier() const;
    void afficherInfos() const;
};

#endif
