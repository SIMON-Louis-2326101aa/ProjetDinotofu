#include <iostream>
#include <algorithm>
#include "entite/Joueur.hpp"
#include "items/arme/Arme.hpp"
#include "items/consommable/Consommable.hpp"

Joueur::Joueur()
: Entite("", "", "", 0, 0, 0, 0, 0),
  niveau(1), experience(0), or_(0), armeEquipeeIdx(-1) {}

Joueur::Joueur(const std::string& nom, const std::string& race, const std::string& classe,
               int pv, int pvMax, int dgtMin, int dgtMax, int dgtCrit,
               int niveau_, int experience_, int orInitial)
: Entite(nom, race, classe, pv, pvMax, dgtMin, dgtMax, dgtCrit),
  niveau(niveau_), experience(experience_), or_(orInitial), armeEquipeeIdx(-1) {}

int  Joueur::getNiveau() const { return niveau; }
int  Joueur::getExperience() const { return experience; }
int  Joueur::getOr() const { return or_; }

void Joueur::gagnerXP(int xp){ if (xp>0) experience += xp; }
void Joueur::gagnerOr(int qte){ if (qte>0) or_ += qte; }
bool Joueur::depenserOr(int qte){ if (qte<=0) return true; if (or_<qte) return false; or_-=qte; return true; }

const std::vector<Arme>&        Joueur::getArmes()  const { return armes; }
const std::vector<Consommable>& Joueur::getConsos() const { return consos; }
void Joueur::ajouterArme(const Arme& a){ armes.push_back(a); }
void Joueur::ajouterConsommable(const Consommable& c){ consos.push_back(c); }

int Joueur::getArmeEquipeeIndex() const { return armeEquipeeIdx; }
bool Joueur::equiperArme(int index){
    if (index<0 || index>= (int)armes.size()) return false;
    if (armeEquipeeIdx>=0){ armes[armeEquipeeIdx].retirerDe(*this); armeEquipeeIdx=-1; }
    armes[index].appliquerSur(*this); armeEquipeeIdx=index; return true;
}
bool Joueur::desequiperArme(){
    if (armeEquipeeIdx<0) return false;
    armes[armeEquipeeIdx].retirerDe(*this); armeEquipeeIdx=-1; return true;
}

bool Joueur::utiliserConsommable(int index, int& bonusDegatsTemp){
    if (index<0 || index>=(int)consos.size()) return false;
    if (!consos[index].consommer(*this, bonusDegatsTemp)) return false;
    consos.erase(consos.begin()+index);
    return true;
}

void Joueur::afficherInfos() const {
    std::cout << "[Joueur] ";
    afficher();
    std::cout << "  Niveau: " << niveau << " | XP: " << experience << " | Or: " << or_ << "\n";
    if (armeEquipeeIdx>=0) std::cout << "  Arme équipée: #" << armeEquipeeIdx << "\n";
    else std::cout << "  Aucune arme équipée\n";
}

void Joueur::afficherInventaire() const {
    std::cout << "=== Armes (" << armes.size() << ") ===\n";
    for (size_t i=0;i<armes.size();++i){ std::cout << "  ["<<i<<"] "; armes[i].afficher(); }
    std::cout << "=== Consommables (" << consos.size() << ") ===\n";
    for (size_t i=0;i<consos.size();++i){ std::cout << "  ["<<i<<"] "; consos[i].afficher(); }
}
