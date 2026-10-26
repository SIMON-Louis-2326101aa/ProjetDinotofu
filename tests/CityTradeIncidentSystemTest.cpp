#include "world/CityTradeFlowSystem.hpp"
#include "world/CityTradeIncidentSystem.hpp"

#include <cassert>
#include <iostream>

int main()
{
    int activityWindows = 0;
    int incidentWindows = 0;
    int firstIncidentDay = -1;
    CityTradeRouteActivity firstIncident;

    for (int day = 0; day < 500; ++day)
    {
        const CityTradeRouteActivity route = CityTradeFlowSystem::routeBetween(
            "rocheveille", "port_lanterne", day, ShopType::Weapon,
            "stable_market", "stable_market");
        if (!route.supported || (!route.departing && !route.arriving && !route.disrupted)) continue;
        ++activityWindows;
        if (!route.incidentActive) continue;

        ++incidentWindows;
        if (firstIncidentDay < 0)
        {
            firstIncidentDay = day;
            firstIncident = route;
        }

        assert(!route.incidentId.empty());
        assert(!route.incidentHeadline.empty());
        assert(!route.incidentSummary.empty());
        assert(route.incidentSeverity >= 1 && route.incidentSeverity <= 3);
        if (route.incidentEscortOpportunity)
        {
            assert(route.incidentStockModifierPercent == 0);
        }
        else
        {
            assert(route.incidentStockModifierPercent <= 0);
        }
    }

    assert(activityWindows > 20);
    assert(incidentWindows > 0);
    assert(incidentWindows < activityWindows / 2); // incidents stay rarer than normal active traffic.
    assert(firstIncidentDay >= 0);

    // The destination economy must see the same incident instead of rolling a second unrelated one.
    const CityTradePressure pressure = CityTradeFlowSystem::pressureFor(
        "port_lanterne", firstIncidentDay, ShopType::Weapon, "stable_market");
    if (firstIncident.arriving || firstIncident.disrupted)
    {
        assert(pressure.incidentActive);
        assert(pressure.incidentId == firstIncident.incidentId);
    }

    // No route => no invented world incident.
    const CityTradeRouteActivity invalid = CityTradeFlowSystem::routeBetween(
        "rocheveille", "rocheveille", firstIncidentDay, ShopType::Weapon,
        "stable_market", "stable_market");
    assert(!invalid.supported);
    assert(!invalid.incidentActive);

    // Service-only categories do not suddenly create physical cargo accidents.
    const CityTradeIncident serviceIncident = CityTradeIncidentSystem::evaluate(
        "rocheveille", "port_lanterne", firstIncidentDay, ShopType::Lodging,
        60, true, true, false, false);
    assert(!serviceIncident.active);

    std::cout << "CityTradeIncidentSystemTest OK\n";
    return 0;
}
