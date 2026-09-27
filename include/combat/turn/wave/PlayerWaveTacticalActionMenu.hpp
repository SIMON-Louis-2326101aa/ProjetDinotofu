#ifndef INCLUDE_COMBAT_TURN_WAVE_PLAYERWAVETACTICALACTIONMENU_HPP
#define INCLUDE_COMBAT_TURN_WAVE_PLAYERWAVETACTICALACTIONMENU_HPP

#include "core/Random.hpp"
#include "entity/Player.hpp"
#include "combat/EnemyCombatQueue.hpp"

namespace PlayerWaveTacticalActionMenu
{
    bool open(Player& player, EnemyCombatQueue& wave, Random& random);
}

#endif
