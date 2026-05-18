#ifndef SYSTEME_DEGATS_HPP
#define SYSTEME_DEGATS_HPP

#include "combat/RapportDegats.hpp"
#include "entite/Entite.hpp"

class SystemeDegats
{
public:
    static RapportDegats calculerDegatsRecus(Entite& defenseur, int degatsBruts);
    static void afficherRapportDegats(const Entite& defenseur, const RapportDegats& rapport);

    static int appliquerProtectionArmure(Entite& defenseur, int degatsBruts);
};

#endif