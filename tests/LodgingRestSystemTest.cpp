#include "entity/Player.hpp"
#include "world/LodgingRestSystem.hpp"

#include <cassert>
#include <iostream>

int main()
{
    Player player;
    player.setCurrentCityId("valebrume");

    for (int i = 0; i < 3; ++i)
    {
        const StableFloorAccess access = LodgingRestSystem::requestStableFloorRest(player, "valebrume");
        assert(access.allowed);
        assert(!access.banned);
        player.advanceWorldDays(1);
    }

    const StableFloorAccess overuse = LodgingRestSystem::requestStableFloorRest(player, "valebrume");
    assert(!overuse.allowed);
    assert(overuse.evictedForOveruse);
    assert(overuse.banned);
    assert(overuse.banDaysRemaining == 3);

    const StableFloorAccess sameDay = LodgingRestSystem::stableFloorAccess(player, "valebrume");
    assert(!sameDay.allowed);
    assert(sameDay.banned);
    assert(sameDay.banDaysRemaining == 3);

    player.advanceWorldDays(2);
    const StableFloorAccess stillBanned = LodgingRestSystem::stableFloorAccess(player, "valebrume");
    assert(stillBanned.banned);
    assert(stillBanned.banDaysRemaining == 1);

    // The ban is local: another city's stable can still be used.
    const StableFloorAccess otherCity = LodgingRestSystem::stableFloorAccess(player, "rocheveille");
    assert(otherCity.allowed);

    player.advanceWorldDays(1);
    const StableFloorAccess banExpired = LodgingRestSystem::stableFloorAccess(player, "valebrume");
    assert(banExpired.allowed);
    assert(!banExpired.banned);

    // The rolling window also cools down naturally; old free nights do not poison the city forever.
    player.advanceWorldDays(4);
    const StableFloorAccess cooledDown = LodgingRestSystem::stableFloorAccess(player, "valebrume");
    assert(cooledDown.allowed);
    assert(cooledDown.recentFreeNights == 0);

    std::cout << "LodgingRestSystemTest OK\n";
    return 0;
}
