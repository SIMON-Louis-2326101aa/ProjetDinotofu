#ifndef SYSTEME_CLASSES_COMBAT_HPP
#define SYSTEME_CLASSES_COMBAT_HPP

#include "entite/Entite.hpp"

#include <string>

class SystemeClassesCombat
{
public:
    static int getChanceFuiteBase(const Entite& entite);
    static int getReductionDegatsBasePourcentage(const Entite& entite);

private:
    static std::string normaliserTexteClasse(const std::string& texteClasse);
};

#endif