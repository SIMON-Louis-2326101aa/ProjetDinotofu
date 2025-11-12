#include <iostream>
#include <algorithm>
#include "entite/monstre/Monstre.hpp"

Monstre::Monstre()
: Entite("", "", "Monstre", 0, 0, 0, 0, 0), type("Inconnu"), butinOr(0) {}

Monstre::Monstre(const std::string& nom, const std::string& type_,
                 int pv, int pvMax, int dgtMin, int dgtMax, int dgtCrit,
                 int butinOr_)
: Entite(nom, /*race*/"", /*classe*/"Monstre", pv, pvMax, dgtMin, dgtMax, dgtCrit),
  type(type_), butinOr(std::max(0, butinOr_)) {}

const std::string& Monstre::getType() const { return type; }
int Monstre::getButinOr() const { return butinOr; }
void Monstre::setType(const std::string& t) { type = t; }
void Monstre::setButinOr(int v) { butinOr = std::max(0, v); }

void Monstre::crier() const {
    std::cout << "[" << type << "] " << getNom() << " rugit !\n";
}

void Monstre::afficherInfos() const {
    std::cout << "[Monstre] ";
    afficher(); // de Entite
    std::cout << "  Type: " << type
              << " | Butin: " << butinOr << " or\n";
}
