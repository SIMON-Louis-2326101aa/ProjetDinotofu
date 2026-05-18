#include "combat/BossCombat.hpp"

#include "combat/boss/BossDecryptage.hpp"
#include "combat/boss/BossUltime.hpp"
#include "combat/boss/BossFinTour.hpp"

void BossCombat::verifierDecryptageBoss(Boss& boss)
{
    BossDecryptage::verifierDecryptageBoss(boss);
}

void BossCombat::executerUltimeBoss(
    Boss& boss,
    Entite& joueur,
    Random& random
)
{
    BossUltime::executerUltimeBoss(
        boss,
        joueur,
        random
    );
}

bool BossCombat::gererFinTourBoss(
    Boss& boss,
    Entite& joueur
)
{
    return BossFinTour::gererFinTourBoss(
        boss,
        joueur
    );
}