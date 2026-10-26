#include "economy/shop/ShopCatalog.hpp"
#include "world/CityStockDynamicsSystem.hpp"

#include <cassert>
#include <iostream>

namespace
{
    int finiteStockTotal(const ShopInventory& shop)
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
    // Same global shop catalog, different city: specialization must change real public stock.
    ShopInventory rocheveilleWeapons = ShopCatalog::createPreviewShop(ShopType::Weapon);
    ShopInventory portWeapons = ShopCatalog::createPreviewShop(ShopType::Weapon);
    CityStockDynamicsSystem::applyToShop("rocheveille", 17, 0, 0, rocheveilleWeapons);
    CityStockDynamicsSystem::applyToShop("port_lanterne", 17, 0, 0, portWeapons);
    assert(finiteStockTotal(rocheveilleWeapons) > finiteStockTotal(portWeapons));

    // A raid preparation creates real pressure on public military stock, but Rocheveille's
    // reference-level forge identity cushions the shortage instead of being erased by wealth logic.
    ShopInventory stableRocheveilleWeapons = ShopCatalog::createPreviewShop(ShopType::Weapon);
    ShopInventory raidRocheveilleWeapons = ShopCatalog::createPreviewShop(ShopType::Weapon);
    const auto stable = CityStockDynamicsSystem::applyToShop("rocheveille", 17, 0, 0, stableRocheveilleWeapons);
    const auto raid = CityStockDynamicsSystem::applyToShop("rocheveille", 61, 0, 0, raidRocheveilleWeapons);
    assert(stable.specializationStrength == 3);
    assert(raid.stockModifierPercent < stable.stockModifierPercent);
    assert(finiteStockTotal(raidRocheveilleWeapons) <= finiteStockTotal(stableRocheveilleWeapons));
    assert(CityStockDynamicsSystem::availabilityLine("rocheveille", 61, 0, 0, ShopType::Weapon).find("stock") != std::string::npos);

    // Merchant fairs genuinely broaden public material availability instead of only changing prices.
    ShopInventory stablePortMaterials = ShopCatalog::createPreviewShop(ShopType::Material);
    ShopInventory fairPortMaterials = ShopCatalog::createPreviewShop(ShopType::Material);
    CityStockDynamicsSystem::applyToShop("port_lanterne", 17, 0, 0, stablePortMaterials);
    CityStockDynamicsSystem::applyToShop("port_lanterne", 14, 0, 0, fairPortMaterials);
    assert(finiteStockTotal(fairPortMaterials) >= finiteStockTotal(stablePortMaterials));

    std::cout << "CityStockDynamicsSystemTest OK\n";
    return 0;
}
