#pragma once

#include "economy/shop/ShopType.hpp"

#include <string>
#include <vector>

struct CityTradePressure
{
    ShopType goodsType = ShopType::Unknown;
    std::string sourceCityId;
    std::string sourceCityName;
    std::string destinationCityId;
    std::string destinationCityName;
    int distance = 0;
    int importStockModifierPercent = 0;
    int exportStockModifierPercent = 0;
    int netStockModifierPercent = 0;
    bool incomingConvoy = false;
    bool outgoingConvoy = false;
    bool disrupted = false;
    bool emergencyPriority = false;
    bool incidentActive = false;
    std::string incidentId;
    std::string incidentHeadline;
    int incidentSeverity = 0;
    int incidentStockModifierPercent = 0;
    bool incidentEscortOpportunity = false;
    bool incidentCreatesBountyLead = false;
    std::string status;
};

struct CityTradeRouteActivity
{
    ShopType goodsType = ShopType::Unknown;
    std::string sourceCityId;
    std::string sourceCityName;
    std::string destinationCityId;
    std::string destinationCityName;
    int distance = 0;
    bool supported = false;
    bool departing = false;
    bool arriving = false;
    bool disrupted = false;
    bool emergencyPriority = false;
    bool incidentActive = false;
    std::string incidentId;
    std::string incidentHeadline;
    int incidentSeverity = 0;
    int incidentStockModifierPercent = 0;
    bool incidentEscortOpportunity = false;
    bool incidentCreatesBountyLead = false;
    std::string incidentSummary;
    std::string status;
};

namespace CityTradeFlowSystem
{
    CityTradePressure pressureFor(
        const std::string& cityId,
        int worldDay,
        ShopType goodsType,
        const std::string& localEventId = ""
    );

    // Describes one explicit commercial route instead of only the aggregate pressure seen by
    // one city. This is used by systems such as bounty pursuit without creating fake convoys.
    CityTradeRouteActivity routeBetween(
        const std::string& sourceCityId,
        const std::string& destinationCityId,
        int worldDay,
        ShopType goodsType,
        const std::string& sourceEventId = "",
        const std::string& destinationEventId = ""
    );

    std::vector<std::string> informationLines(
        const std::string& cityId,
        int worldDay,
        const std::string& localEventId = ""
    );
}
