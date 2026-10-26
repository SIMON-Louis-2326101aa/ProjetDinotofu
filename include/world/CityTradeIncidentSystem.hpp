#pragma once

#include "economy/shop/ShopType.hpp"

#include <string>

enum class CityTradeIncidentKind
{
    None,
    EscortRequest,
    CargoTheft,
    MissingConvoy,
    DamagedArrival,
    RoadObstruction
};

struct CityTradeIncident
{
    bool active = false;
    CityTradeIncidentKind kind = CityTradeIncidentKind::None;
    std::string id;
    std::string headline;
    std::string summary;
    int severity = 0; // 1..3 when active.
    int destinationStockModifierPercent = 0;
    bool escortOpportunity = false;
    bool createsBountyLead = false;
};

namespace CityTradeIncidentSystem
{
    // Deterministic, rare incident layer evaluated only on a route that is currently
    // departing, arriving or already disrupted. Normal traffic remains normal most days.
    CityTradeIncident evaluate(
        const std::string& sourceCityId,
        const std::string& destinationCityId,
        int worldDay,
        ShopType goodsType,
        int distance,
        bool departing,
        bool arriving,
        bool disrupted,
        bool emergencyPriority
    );

    std::string kindId(CityTradeIncidentKind kind);
}
