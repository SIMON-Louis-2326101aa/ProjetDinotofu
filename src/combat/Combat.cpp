#include <iostream>
#include "combat/Combat.hpp"

namespace Combat {

TourLog tourAttaque(Entite& a, Entite& d, int& bonus, bool aJoueur, std::ostream& out){
    auto res = executerAttaque(a, d, bonus);
    out << (aJoueur ? "[Joueur]" : "[Monstre]") << " "; a.afficher();
    out << "   => attaque -> " 
        << (res.issue==IssueAttaque::Crit?"CRIT":res.issue==IssueAttaque::Hit?"HIT":"MISS")
        << " pour " << res.degatsInfliges << " dmg\n";
    out << "   > Défenseur après coup: "; d.afficher();
    bonus = 0;
    return {res.issue, res.degatsInfliges, aJoueur};
}

DuelResult duel(Joueur& j, Monstre& m, bool joueurCommence, std::ostream& out){
    int tours=0, bJ=0, bM=0;
    while (j.estVivant() && m.estVivant()){
        ++tours;
        if (joueurCommence){
            tourAttaque(j, m, bJ, true, out);  if (!m.estVivant()) break;
            tourAttaque(m, j, bM, false, out);
        } else {
            tourAttaque(m, j, bM, false, out); if (!j.estVivant()) break;
            tourAttaque(j, m, bJ, true, out);
        }
    }
    return { j.estVivant(), m.estVivant(), tours };
}

}
