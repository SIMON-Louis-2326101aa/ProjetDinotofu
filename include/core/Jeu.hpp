#ifndef JEU_HPP
#define JEU_HPP

#include <string>

#include "entite/Joueur.hpp"
#include "core/ModeJeu.hpp"

class Jeu
{
private:
    std::string nomJoueur;
    Joueur joueurPrincipal;
    ModeJeu modeSelectionne;

    void afficherIntroduction();
    void demanderNomJoueur();
    void choisirClasseJoueur();
    void choisirModeDeJeu();
    void afficherModeSelectionne();
    void lancerModeSelectionne();

public:
    Jeu();
    void lancer();
};

#endif