#include <iostream>
#include "entite/Entite.hpp"

// --- Constructeurs ---
Entite::Entite()
    : nom(""), race(""), classe(""),
      pv(0), pvMax(0), dgtMin(0), dgtMax(0), dgtCrit(0) {}

Entite::Entite(const std::string& nom, const std::string& race, const std::string& classe,
               int pv, int pvMax, int dgtMin, int dgtMax, int dgtCrit)
    : nom(nom), race(race), classe(classe),
      pv(pv), pvMax(pvMax), dgtMin(dgtMin), dgtMax(dgtMax), dgtCrit(dgtCrit) {}

// --- Getters ---
std::string Entite::getNom()   const { return nom; }
std::string Entite::getRace()  const { return race; }
std::string Entite::getClasse()const { return classe; }
int Entite::getPV()            const { return pv; }
int Entite::getPvMax()         const { return pvMax; }
int Entite::getDgtMin()        const { return dgtMin; }
int Entite::getDgtMax()        const { return dgtMax; }
int Entite::getDgtCrit()       const { return dgtCrit; }

// --- Setters ---
void Entite::setNom(const std::string& v)    { nom = v; }
void Entite::setRace(const std::string& v)   { race = v; }
void Entite::setClasse(const std::string& v) { classe = v; }

void Entite::setPV(int newPV) {
    pv = std::clamp(newPV, 0, pvMax);
}

void Entite::setPvMax(int newPvMax) {
    pvMax = std::max(0, newPvMax);
    if (pv > pvMax) pv = pvMax;
}

void Entite::setDgtMin(int newDgtMin) {
    dgtMin = newDgtMin;
    if (dgtMax < dgtMin) dgtMax = dgtMin;
}

void Entite::setDgtMax(int newDgtMax) {
    dgtMax = newDgtMax;
    if (dgtMax < dgtMin) dgtMin = dgtMax;
}

void Entite::setDgtCrit(int newDgtCrit) { dgtCrit = newDgtCrit; }

// --- Méthodes utilitaires ---
bool Entite::estVivant() const { return pv > 0; }

void Entite::prendreDegats(int v) {
    if (v < 0) return;
    pv = std::max(0, pv - v);
}

void Entite::soigner(int v) {
    if (v < 0) return;
    pv = std::min(pvMax, pv + v);
}

void Entite::afficher() const {
    std::cout << "Nom: " << nom
              << " | Race: " << race
              << " | Classe: " << classe
              << " | PV: " << pv << "/" << pvMax
              << " | Dégâts: " << dgtMin << "-" << dgtMax
              << " (Crit: " << dgtCrit << ")\n";
}
