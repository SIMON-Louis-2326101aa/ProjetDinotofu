#ifndef SYSTEME_DEGATS_HPP
#define SYSTEME_DEGATS_HPP

#include "entite/Entite.hpp"

class SystemeDegats
{
public:
    static int appliquerProtectionArmure(Entite& defenseur, int degatsBruts);
};

#endif