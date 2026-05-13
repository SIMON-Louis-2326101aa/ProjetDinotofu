#ifndef SYSTEME_FUITE_HPP
#define SYSTEME_FUITE_HPP

#include "core/Random.hpp"
#include "entite/Joueur.hpp"
#include "entite/Monstre.hpp"
#include "entite/Boss.hpp"

class SystemeFuite
{
public:
    static bool joueurTenteFuite(Joueur& joueur, Random& random);
    static bool joueurTenteFuiteBoss(const Joueur& joueur, const Boss& boss);

    static bool monstrePeutTenterFuite(const Monstre& monstre);
    static bool monstreTenteFuite(Monstre& monstre, Random& random);

private:
    static constexpr int CHANCE_FUITE_JOUEUR = 55;
    static constexpr int CHANCE_FUITE_MONSTRE_FAIBLE = 20;
};

#endif