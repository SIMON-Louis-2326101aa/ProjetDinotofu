#ifndef JOUEUR_HPP
#define JOUEUR_HPP

#include <vector>
#include "entite/Entite.hpp"

class Arme; class Consommable;

class Joueur : public Entite {
private:
    int niveau, experience, or_;
    std::vector<Arme> armes;
    std::vector<Consommable> consos;
    int armeEquipeeIdx; // -1 = aucune

public:
    Joueur();
    Joueur(const std::string& nom, const std::string& race, const std::string& classe,
           int pv, int pvMax, int dgtMin, int dgtMax, int dgtCrit,
           int niveau=1, int experience=0, int orInitial=0);
    virtual ~Joueur() = default;

    int  getNiveau() const;    int  getExperience() const;    int getOr() const;
    void gagnerXP(int xp);      void gagnerOr(int qte);        bool depenserOr(int qte);

    const std::vector<Arme>&        getArmes() const;
    const std::vector<Consommable>& getConsos() const;
    void ajouterArme(const Arme& a);
    void ajouterConsommable(const Consommable& c);

    int  getArmeEquipeeIndex() const;
    bool equiperArme(int index);
    bool desequiperArme();

    bool utiliserConsommable(int index, int& bonusDegatsTemp);

    void afficherInfos() const;
    void afficherInventaire() const;
};

#endif
