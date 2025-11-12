#ifndef ARME_HPP
#define ARME_HPP

#include <string>

class Entite; // fwd decl

class Arme {
private:
    std::string nom;
    int bonusMin;
    int bonusMax;
    int bonusCrit;

public:
    Arme();
    Arme(const std::string& nom, int bonusMin, int bonusMax, int bonusCrit);

    // getters
    const std::string& getNom() const;
    int getBonusMin() const;
    int getBonusMax() const;
    int getBonusCrit() const;

    // setters
    void setNom(const std::string& v);
    void setBonusMin(int v);
    void setBonusMax(int v);
    void setBonusCrit(int v);

    // équipement
    void appliquerSur(Entite& e) const;
    void retirerDe(Entite& e) const;

    // affichage (implémenté dans .cpp)
    void afficher() const;
};

#endif
