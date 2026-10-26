#include "world/CityTradeFlowSystem.hpp"
#include "world/CityEconomyDynamicsSystem.hpp"
#include "world/CityStockDynamicsSystem.hpp"
#include "economy/shop/ShopCatalog.hpp"

#include <cassert>
#include <iostream>
#include <string>

namespace
{
    int totalFiniteStock(const ShopInventory& shop)
    {
        int total = 0;
        for (const ShopItem& item : shop.getItems())
        {
            if (item.getStock() > 0) total += item.getStock();
        }
        return total;
    }
}

int main()
{
    // Rocheveille is a reference military producer: it must sometimes export weapons.
    bool foundWeaponExport = false;
    for (int day = 0; day < 40; ++day)
    {
        const auto flow = CityTradeFlowSystem::pressureFor("rocheveille", day, ShopType::Weapon, "stable_market");
        if (flow.outgoingConvoy)
        {
            foundWeaponExport = true;
            assert(flow.exportStockModifierPercent < 0);
            assert(flow.netStockModifierPercent <= 0 || flow.incomingConvoy);
            break;
        }
    }
    assert(foundWeaponExport);

    // Port-Lanterne is not a weapon specialist and must sometimes receive military imports.
    bool foundWeaponImport = false;
    int importDay = -1;
    for (int day = 0; day < 50; ++day)
    {
        const auto flow = CityTradeFlowSystem::pressureFor("port_lanterne", day, ShopType::Weapon, "stable_market");
        if (flow.incomingConvoy && !flow.disrupted && flow.importStockModifierPercent > 0)
        {
            foundWeaponImport = true;
            importDay = day;
            assert(flow.sourceCityId == "rocheveille");
            break;
        }
    }
    assert(foundWeaponImport);

    // Other systems can query the exact same real route instead of inventing a convenient caravan.
    const auto explicitRoute = CityTradeFlowSystem::routeBetween(
        "rocheveille", "port_lanterne", importDay, ShopType::Weapon, "stable_market", "stable_market");
    assert(explicitRoute.supported);
    assert(explicitRoute.sourceCityId == "rocheveille");
    assert(explicitRoute.destinationCityId == "port_lanterne");
    assert(explicitRoute.arriving || explicitRoute.departing || explicitRoute.disrupted);

    // Real stocks must feel the arrival, not just the information text.
    ShopInventory withTrade = ShopCatalog::createPreviewShop(ShopType::Weapon);
    ShopInventory withoutTrade = ShopCatalog::createPreviewShop(ShopType::Weapon);
    const CityEconomicPulse pulse = CityEconomyDynamicsSystem::stateFor("port_lanterne", importDay, 0, 0);
    const auto tradeAdjustment = CityStockDynamicsSystem::adjustmentFor("port_lanterne", importDay, pulse, ShopType::Weapon);
    const auto noTradeAdjustment = CityStockDynamicsSystem::adjustmentFor("port_lanterne", pulse, ShopType::Weapon);
    assert(tradeAdjustment.tradeStockModifierPercent > 0);
    CityStockDynamicsSystem::applyToShop("port_lanterne", importDay, 0, 0, withTrade);

    // Rebuild manually from the no-trade percentage to prove the arrival affects the real stock path.
    for (ShopItem& item : withoutTrade.getMutableItems())
    {
        const int oldStock = item.getStock();
        if (oldStock <= 0) continue;
        int target = oldStock * (100 + noTradeAdjustment.stockModifierPercent) / 100;
        if (target < 0) target = 0;
        while (item.getStock() > target && item.consumeOneStock()) {}
        if (item.getStock() >= 0 && item.getStock() < target) item.addStock(target - item.getStock());
    }
    assert(totalFiniteStock(withTrade) >= totalFiniteStock(withoutTrade));

    // An emergency must suspend outbound critical supplies instead of exporting the city's last swords.
    const auto emergency = CityTradeFlowSystem::pressureFor(
        "rocheveille", importDay, ShopType::Weapon, "raid_defense_mobilization");
    assert(!emergency.outgoingConvoy);

    // The local information desk can expose routes and convoy status.
    bool foundTradeInfo = false;
    for (int day = 0; day < 50 && !foundTradeInfo; ++day)
    {
        const auto lines = CityTradeFlowSystem::informationLines("port_lanterne", day, "stable_market");
        for (const std::string& line : lines)
        {
            if (line.find("arrivage") != std::string::npos || line.find("convoi") != std::string::npos || line.find("expédition") != std::string::npos)
            {
                foundTradeInfo = true;
                break;
            }
        }
    }
    assert(foundTradeInfo);

    std::cout << "CityTradeFlowSystemTest OK\n";
    return 0;
}
