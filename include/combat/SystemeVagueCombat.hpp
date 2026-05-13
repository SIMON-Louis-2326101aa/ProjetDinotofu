#ifndef SYSTEME_VAGUE_COMBAT_HPP
#define SYSTEME_VAGUE_COMBAT_HPP

#include "combat/FileEnnemisCombat.hpp"

class SystemeVagueCombat
{
public:
    static FileEnnemisCombat creerVagueDemo();
    static void afficherIntroductionVague();
    static void afficherArriveePremiereLigne(const FileEnnemisCombat& vague);
};

#endif