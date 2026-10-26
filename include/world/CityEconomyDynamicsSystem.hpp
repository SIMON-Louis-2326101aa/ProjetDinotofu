#pragma once

#include "economy/shop/ShopType.hpp"
#include "world/CityProfileSystem.hpp"

#include <string>
#include <vector>

struct CityMarketDemand
{
    ShopType goodsType = ShopType::Unknown;
    std::string label;
    int sellBonusPercent = 0;
};

struct CityEconomicPulse
{
    CityWealthTier structuralWealth = CityWealthTier::Comfortable;
    CityWealthTier effectiveWealth = CityWealthTier::Comfortable;
    int prosperityShift = 0;
    int generalBuyModifierPercent = 0;
    int generalSellModifierPercent = 0;
    int serviceQualityDelta = 0;
    int remainingDays = 0;
    bool temporary = false;
    std::string eventId;
    std::string headline;
    std::vector<CityMarketDemand> demands;
};

namespace CityEconomyDynamicsSystem
{
    // Temporary city economy derived from the structural city profile, the weekly city rhythm
    // and actual repair/crisis state. The structural wealth is never overwritten by a temporary pulse.
    CityEconomicPulse stateFor(
        const std::string& cityId,
        int worldDay,
        int repairDaysRemaining,
        int recentEventDays = 0
    );

    int demandSellBonusPercent(const CityEconomicPulse& state, ShopType goodsType);
    int adjustedServiceQuality(int structuralQuality, const CityEconomicPulse& state);

    // Combines the structural city specialization/wealth with the current economic pulse.
    // This is the shared reference score future artisan/service menus should use.
    int serviceQualityFor(
        const std::string& cityId,
        int worldDay,
        int repairDaysRemaining,
        int recentEventDays,
        ShopType serviceType
    );

    std::vector<std::string> informationLines(
        const std::string& cityId,
        int worldDay,
        int repairDaysRemaining,
        int recentEventDays = 0
    );
}
