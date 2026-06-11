// English: Consumes blessings to resolve exceptional lethal-survival effects.
// Français : Consume les bénédictions pour résoudre les survies exceptionnelles en mode léthal.
#include "progression/blessing/BlessingSystem.hpp"

#include "progression/blessing/SurvivalAnomaly.hpp"
#include "entity/Player.hpp"

bool BlessingSystem::tryTriggerLethalSurvival(Player& player)
{
    if (!SurvivalAnomaly::canApply(player))
    {
        return false;
    }

    SurvivalAnomaly::apply(player);
    return !player.isDead();
}
