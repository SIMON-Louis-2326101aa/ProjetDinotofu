#include <iostream>
#include "items/consommable/Consommable.hpp"
#include "entite/Entite.hpp"

Consommable::Consommable() : nom(""), type(TypeConsommable::Soin), valeur(0) {}

Consommable::Consommable(const std::string& n, TypeConsommable t, int v)
    : nom(n), type(t), valeur(v) {}

const std::string& Consommable::getNom() const { return nom; }
TypeConsommable Consommable::getType() const { return type; }
int Consommable::getValeur() const { return valeur; }

void Consommable::setNom(const std::string& v) { nom = v; }
void Consommable::setType(TypeConsommable t) { type = t; }
void Consommable::setValeur(int v) { valeur = v; }

bool Consommable::consommer(Entite& cible, int& bonusDegatsTemp) const {
    if (valeur <= 0) return false;

    switch (type) {
        case TypeConsommable::Soin:
            cible.soigner(valeur);
            return true;
        case TypeConsommable::BonusDegats:
            bonusDegatsTemp += valeur;
            return true;
        default:
            return false;
    }
}

void Consommable::afficher() const {
    std::cout << "Conso: " << nom << " | Type: "
              << (type == TypeConsommable::Soin ? "Soin" : "BonusDegats")
              << " | Valeur: " << valeur << "\n";
}
