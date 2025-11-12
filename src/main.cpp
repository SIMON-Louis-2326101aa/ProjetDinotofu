#include <iostream>
#include "entite/Joueur.hpp"
#include "entite/monstre/Monstre.hpp"
#include "items/Arme.hpp"
#include "items/Consommable.hpp"
#include "action/Action.hpp"
#include "combat/Combat.hpp"

int main() {
    using namespace std;

    // Joueur de test
    Joueur j("Hazack", "Humain", "Assassin", 100, 100, 5, 12, 20, 1, 0, 50);
    j.ajouterArme(Arme("Dague", 2, 4, 6));
    j.equiperArme(0);

    // Monstre de test
    Monstre m("Garde de pierre", "Protecteur", 120, 120, 4, 10, 18, 25);

    cout << "=== Etat initial ===\n";
    j.afficherInfos();
    m.afficherInfos();

    cout << "\n=== Début du duel ===\n";
    auto res = Combat::duel(j, m, /*joueurCommence=*/true, cout);

    cout << "\n=== Résultat ===\n";
    cout << (res.joueurVivant ? "Le joueur est vivant." : "Le joueur est KO.") << "\n";
    cout << (res.monstreVivant ? "Le monstre est vivant." : "Le monstre est KO.") << "\n";
    cout << "Tours joués: " << res.toursJoues << "\n";
    return 0;
}
