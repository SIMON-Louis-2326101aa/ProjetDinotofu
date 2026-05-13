#ifndef IA_COMBAT_HPP
#define IA_COMBAT_HPP

#include "entite/Entite.hpp"
#include "entite/Boss.hpp"
#include "core/Random.hpp"

class IACombat
{
public:
    static int choisirActionIA(const Entite& ia, Random& random);
    static int choisirActionBoss(const Boss& boss, Random& random);

private:
    static bool peutUtiliserPotionSoin(const Entite& entite);
    static bool peutUtiliserPotionDegats(const Entite& entite);
};

#endif