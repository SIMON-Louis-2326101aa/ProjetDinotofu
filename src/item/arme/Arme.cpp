#include <iostream>
#include <algorithm>
#include "items/arme/Arme.hpp"
#include "entite/Entite.hpp"

Arme::Arme() : nom(""), bonusMin(0), bonusMax(0), bonusCrit(0) {}

Arme::Arme(const std::string& n, int bMin, int bMax, int bCrit)
    : nom(n), bonusMin(bMin), bonusMax(bMax), bonusCrit(bCrit) {}

const std::string& Arme::getNom() const { return nom; }
int Arme::getBonusMin() const { return bonusMin; }
int Arme::getBonusMax() const { return bonusMax; }
int Arme::getBonusCrit() const { return bonusCrit; }

void Arme::setNom(const std::string& v) { nom = v; }
void Arme::setBonusMin(int v) { bonusMin = v; }
void Arme::setBonusMax(int v) { bonusMax = v; }
void Arme::setBonusCrit(int v) { bonusCrit = v; }

void Arme::appliquerSur(Entite& e) const {
    e.setDgtMin(e.getDgtMin() + bonusMin);
    e.setDgtMax(std::max(e.getDgtMin(), e.getDgtMax() + bonusMax));
    e.setDgtCrit(e.getDgtCrit() + bonusCrit);
}

void Arme::retirerDe(Entite& e) const {
    e.setDgtMin(e.getDgtMin() - bonusMin);
    e.setDgtMax(e.getDgtMax() - bonusMax);
    e.setDgtCrit(e.getDgtCrit() - bonusCrit);
}

void Arme::afficher() const {
    std::cout << "Arme: " << nom
              << " | Bonus DGT: +" << bonusMin << " à +" << bonusMax
              << " (crit +" << bonusCrit << ")\n";
}
