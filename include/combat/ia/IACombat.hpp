#ifndef IA_COMBAT_HPP
#define IA_COMBAT_HPP

#include "combat/ia/ActionIA.hpp"

#include "core/Random.hpp"

#include "entite/Entite.hpp"
#include "entite/Boss.hpp"

class IACombat
{
public:
    static ActionIA choisirActionIA(const Entite& ia, Random& random);
    static ActionIA choisirActionBoss(const Boss& boss, Random& random);

private:
    static bool peutUtiliserPotionSoin(const Entite& entite);
    static bool peutUtiliserPotionDegats(const Entite& entite);

    static int calculerPourcentagePv(const Entite& entite);

    static ActionIA choisirActionIAEnDanger(
        bool soinDisponible,
        bool degatsDisponible,
        int tirage
    );

    static ActionIA choisirActionIAMiVie(
        bool soinDisponible,
        bool degatsDisponible,
        int tirage
    );

    static ActionIA choisirActionIAStable(
        bool degatsDisponible,
        int tirage
    );

    static ActionIA choisirActionBossEnDanger(
        bool soinDisponible,
        bool degatsDisponible,
        bool ultimeDisponible,
        int tirage
    );

    static ActionIA choisirActionBossUltimeDisponible(
        bool degatsDisponible,
        int tirage
    );

    static ActionIA choisirActionBossStable(
        bool degatsDisponible,
        int tirage
    );
};

#endif