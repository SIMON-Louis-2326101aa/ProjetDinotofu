#include <iostream>
#include <algorithm>
#include "entite/pnj/PNJ.hpp"
#include "items/Arme.hpp"
#include "items/Consommable.hpp"
#include "entite/Joueur.hpp"

PNJ::PNJ()
: Entite("", "", "PNJ", 1, 1, 0, 0, 0), role(RolePNJ::Neutre) {}

PNJ::PNJ(const std::string& nom, RolePNJ r)
: Entite(nom, "", "PNJ", 1, 1, 0, 0, 0), role(r) {}

RolePNJ PNJ::getRole() const { return role; }
void PNJ::setRole(RolePNJ r) { role = r; }

void PNJ::ajouterDialogue(const std::string& ligne) {
    dialogues.push_back(ligne);
}
const std::vector<std::string>& PNJ::getDialogues() const { return dialogues; }

void PNJ::ajouterOffreArme(const Arme& arme, int prix) {
    stockArmes.push_back(arme);
    // l’adresse reste stable car vector peut réallouer, donc on référence l’élément après push_back
    // On reprend l’adresse via back()
    offresArmes.push_back(OffreArme{ &stockArmes.back(), std::max(0, prix) });
}

void PNJ::ajouterOffreConsommable(const Consommable& conso, int prix) {
    stockConsos.push_back(conso);
    offresConsos.push_back(OffreConso{ &stockConsos.back(), std::max(0, prix) });
}

const std::vector<OffreArme>& PNJ::getOffresArmes() const { return offresArmes; }
const std::vector<OffreConso>& PNJ::getOffresConsos() const { return offresConsos; }

bool PNJ::vendreArme(int idx, Joueur& acheteur) {
    if (idx < 0 || idx >= (int)offresArmes.size()) return false;
    OffreArme& off = offresArmes[idx];
    if (!acheteur.depenserOr(off.prix)) return false;  // pas assez d’or
    acheteur.ajouterArme(*off.ref);
    // on garde l’offre; tu peux aussi la retirer si stock limité :
    // offresArmes.erase(offresArmes.begin()+idx);
    return true;
}

bool PNJ::vendreConsommable(int idx, Joueur& acheteur) {
    if (idx < 0 || idx >= (int)offresConsos.size()) return false;
    OffreConso& off = offresConsos[idx];
    if (!acheteur.depenserOr(off.prix)) return false;
    acheteur.ajouterConsommable(*off.ref);
    // idem : peut être retiré du stock si tu veux de la rareté
    // offresConsos.erase(offresConsos.begin()+idx);
    return true;
}

void PNJ::afficherDialogues() const {
    if (dialogues.empty()) { std::cout << getNom() << ": ...\n"; return; }
    std::cout << getNom() << " — Dialogues:\n";
    for (const auto& l : dialogues) std::cout << "  « " << l << " »\n";
}

void PNJ::afficherBoutique() const {
    std::cout << getNom() << " — Boutique:\n";
    std::cout << "  Armes:\n";
    for (size_t i=0;i<offresArmes.size();++i) {
        std::cout << "   ["<<i<<"] " << offresArmes[i].prix << " or — ";
        offresArmes[i].ref->afficher();
    }
    std::cout << "  Consommables:\n";
    for (size_t i=0;i<offresConsos.size();++i) {
        std::cout << "   ["<<i<<"] " << offresConsos[i].prix << " or — ";
        offresConsos[i].ref->afficher();
    }
}
