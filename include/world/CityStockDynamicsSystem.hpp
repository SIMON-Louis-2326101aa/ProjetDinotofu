#pragma once

#include "economy/shop/ShopInventory.hpp"
#include "world/CityEconomyDynamicsSystem.hpp"

#include <string>

struct CityStockAdjustment
{
    int specializationStrength = 0;
    int stockModifierPercent = 0;
    int tradeStockModifierPercent = 0;
    int finiteItemsTouched = 0;
    std::string reason;
};

namespace CityStockDynamicsSystem
{
    CityStockAdjustment adjustmentFor(
        const std::string& cityId,
        const CityEconomicPulse& pulse,
        ShopType shopType
    );

    CityStockAdjustment adjustmentFor(
        const std::string& cityId,
        int worldDay,
        const CityEconomicPulse& pulse,
        ShopType shopType
    );

    CityStockAdjustment applyToShop(
        const std::string& cityId,
        int worldDay,
        int repairDaysRemaining,
        int recentEventDays,
        ShopInventory& shop
    );

    std::string availabilityLine(
        const std::string& cityId,
        int worldDay,
        int repairDaysRemaining,
        int recentEventDays,
        ShopType shopType
    );
}
