#pragma once

#include "economy/shop/ShopType.hpp"

#include <string>

struct Quest;

struct BountyTradeTransitOpportunity
{
    bool available = false;
    bool activeConvoy = false;
    bool disruptedConvoy = false;
    ShopType goodsType = ShopType::Unknown;
    std::string goodsLabel;
    int destinationScoreBonus = 0;
    int relocationChanceBonus = 0;
    std::string trailKind;
    int trailReliability = 0;
    std::string summary;
};

namespace BountyTradeTransitSystem
{
    // Evaluates only trade routes that really exist in CityTradeFlowSystem for the current day.
    // A bounty can exploit them, but the same traffic can also leave stronger merchant evidence.
    BountyTradeTransitOpportunity assess(
        const Quest& quest,
        const std::string& sourceCityId,
        const std::string& destinationCityId,
        int worldDay
    );
}
