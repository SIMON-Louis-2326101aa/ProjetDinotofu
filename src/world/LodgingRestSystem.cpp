#include "world/LodgingRestSystem.hpp"

#include "entity/Player.hpp"

#include <algorithm>

namespace
{
    constexpr int kRecentWindowDays = 5;
    constexpr int kToleratedFreeNights = 3;
    constexpr int kBanDays = 3;

    std::string usagePrefix(const std::string& cityId)
    {
        return cityId + ":day:";
    }

    const PlayerJournalRecord* findBan(const Player& player, const std::string& cityId)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == "stable_floor_bans" && record.key == cityId)
            {
                return &record;
            }
        }
        return nullptr;
    }

    int recentUses(const Player& player, const std::string& cityId)
    {
        const int today = std::max(0, player.getWorldDaysElapsed());
        const int oldestDay = std::max(0, today - (kRecentWindowDays - 1));
        const std::string prefix = usagePrefix(cityId);
        int total = 0;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "stable_floor_free_nights") continue;
            if (record.key.rfind(prefix, 0) != 0) continue;
            if (record.lastDay < oldestDay || record.lastDay > today) continue;
            total += std::max(0, record.count);
        }
        return total;
    }
}

StableFloorAccess LodgingRestSystem::stableFloorAccess(const Player& player, const std::string& cityId)
{
    StableFloorAccess result;
    result.recentFreeNights = recentUses(player, cityId);

    if (const PlayerJournalRecord* ban = findBan(player, cityId))
    {
        const int today = std::max(0, player.getWorldDaysElapsed());
        const int elapsed = std::max(0, today - ban->lastDay);
        if (elapsed < kBanDays)
        {
            result.allowed = false;
            result.banned = true;
            result.banDaysRemaining = kBanDays - elapsed;
            result.reason = "L'écurie t'a temporairement interdit le couchage gratuit après des abus répétés.";
            return result;
        }
    }

    if (result.recentFreeNights >= kToleratedFreeNights)
    {
        result.allowed = false;
        result.evictedForOveruse = true;
        result.reason = "L'aubergiste estime que le coin de paille dépanne les voyageurs, mais ne remplace pas un logement permanent.";
    }
    return result;
}

StableFloorAccess LodgingRestSystem::requestStableFloorRest(Player& player, const std::string& cityId)
{
    StableFloorAccess result = stableFloorAccess(player, cityId);
    if (result.banned)
    {
        return result;
    }

    if (result.evictedForOveruse)
    {
        player.recordCanonicalEvent(
            "stable_floor_bans",
            cityId,
            "Interdiction temporaire du couchage gratuit à l'écurie",
            1
        );
        result.banned = true;
        result.banDaysRemaining = kBanDays;
        return result;
    }

    const int today = std::max(0, player.getWorldDaysElapsed());
    player.recordCanonicalEvent(
        "stable_floor_free_nights",
        cityId + ":day:" + std::to_string(today),
        "Nuit gratuite tolérée au fond de l'écurie",
        1
    );
    result.recentFreeNights += 1;
    return result;
}
