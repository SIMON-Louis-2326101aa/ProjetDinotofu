#pragma once

#include <string>

class Player;

struct StableFloorAccess
{
    bool allowed = true;
    bool evictedForOveruse = false;
    bool banned = false;
    int recentFreeNights = 0;
    int banDaysRemaining = 0;
    std::string reason;
};

namespace LodgingRestSystem
{
    // Three free stable-floor nights inside a rolling five-day window are tolerated.
    // A fourth attempt gets the player evicted and starts a three-day local ban.
    StableFloorAccess stableFloorAccess(const Player& player, const std::string& cityId);
    StableFloorAccess requestStableFloorRest(Player& player, const std::string& cityId);
}
