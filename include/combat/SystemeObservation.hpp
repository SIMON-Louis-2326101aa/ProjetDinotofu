#ifndef SYSTEME_OBSERVATION_HPP
#define SYSTEME_OBSERVATION_HPP

#include "entite/Entite.hpp"

class SystemeObservation
{
public:
    static void afficherStatsTerminal(const Entite& cible);
    static void afficherTentativeDecryptageEchouee(const Entite& cible);
};

#endif