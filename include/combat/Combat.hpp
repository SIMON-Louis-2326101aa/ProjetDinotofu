#ifndef COMBAT_HPP
#define COMBAT_HPP

#include <iosfwd>
#include "action/Action.hpp"
#include "entite/Joueur.hpp"
#include "entite/monstre/Monstre.hpp"

namespace Combat {

struct TourLog { IssueAttaque issue; int degats; bool attaquantEtaitJoueur; };
struct DuelResult { bool joueurVivant; bool monstreVivant; int toursJoues; };

TourLog tourAttaque(Entite& attaquant, Entite& defenseur, int& bonusTemp, bool attaquantEstJoueur, std::ostream& out);
DuelResult duel(Joueur& j, Monstre& m, bool joueurCommence, std::ostream& out);

}

#endif
