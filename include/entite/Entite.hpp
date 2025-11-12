#ifndef ENTITE_HPP
#define ENTITE_HPP

#include <string>
#include <algorithm> // std::clamp, std::max

class Entite
{
private:
    std::string nom;
    std::string race;
    std::string classe;
    int pv;
    int pvMax;
    int dgtMin;
    int dgtMax;
    int dgtCrit;

public:
    // --- Constructeurs / Destructeur ---
    Entite(); 
    Entite(const std::string& nom, const std::string& race, const std::string& classe,
           int pv, int pvMax, int dgtMin, int dgtMax, int dgtCrit);
    virtual ~Entite() = default; // si héritage

    // --- Getters ---
    std::string getNom() const;
    std::string getRace() const;
    std::string getClasse() const;
    int getPV() const;
    int getPvMax() const;
    int getDgtMin() const;
    int getDgtMax() const;
    int getDgtCrit() const;

    // --- Setters (avec gardes minimales) ---
    void setNom(const std::string& newNom);
    void setRace(const std::string& newRace);
    void setClasse(const std::string& newClasse);
    void setPV(int newPV);          // clamp [0, pvMax]
    void setPvMax(int newPvMax);    // >= 0 et ajuste pv si besoin
    void setDgtMin(int newDgtMin);  // garde min <= max
    void setDgtMax(int newDgtMax);  // garde min <= max
    void setDgtCrit(int newDgtCrit);

    // --- Méthodes utilitaires ---
    bool estVivant() const;
    void prendreDegats(int v);  // clamp à 0
    void soigner(int v);        // clamp à pvMax
    void afficher() const;      // défini dans .cpp
};

#endif
