// EN: Centralized economy numbers keep related systems from drifting apart.
// FR: Les nombres économiques centralisés évitent que les systèmes se contredisent.
#include "economy/EconomyBalance.hpp"
#include "world/CityProfileSystem.hpp"
#include "economy/shop/ShopType.hpp"

#include <algorithm>

namespace
{
    int cityVaultMultiplier(const std::string& cityId)
    {
        if (cityId == "port_lanterne") return 115;
        if (cityId == "lys_de_givre") return 130;
        if (cityId == "rocheveille") return 105;
        return 100;
    }
}

int EconomyBalance::cityVaultPurchaseCost(const std::string& cityId, int ownedVaultCount, int mayorDiscountTier)
{
    (void)cityId;
    const int safeOwned = std::max(0, ownedVaultCount);

    // V3.51.53: a municipal vault is a major investment. Base price is 2 PP = 2000 PF.
    // Each additional local vault becomes progressively more expensive: +2 PO, then +3 PO, +4 PO, +5 PO...
    // With safeOwned already purchased vaults, cumulative added PO = 2 + 3 + ... + (safeOwned + 1).
    const int cumulativeExtraGoldCoins = safeOwned * (safeOwned + 3) / 2;
    int baseCost = 2000 + cumulativeExtraGoldCoins * 100;

    // Mayor discount applies only to the very first vault, based on real local reputation/services.
    if (safeOwned == 0)
    {
        if (mayorDiscountTier >= 2) return 1250; // exceptional civic recognition
        if (mayorDiscountTier >= 1) return 1500; // strong local trust
    }
    return baseCost;
}

int EconomyBalance::cityVaultSecuritySubscriptionCost(int vaultLevel)
{
    const int safeLevel = std::max(1, std::min(5, vaultLevel));
    // Seven protected days actually spent away from the city. The timer pauses locally.
    static const int weeklyCosts[] = {30, 40, 50, 60, 70};
    return weeklyCosts[safeLevel - 1];
}

int EconomyBalance::cityVaultRecallMessengerSurchargeCopper(const std::string& fromCityId, const std::string& toCityId, int distanceKm)
{
    (void)fromCityId;
    (void)toCityId;
    const int safeDistance = std::max(1, distanceKm);
    // A recall first pays a messenger to carry the request, then the normal transport comes back.
    return std::max(15, 12 + safeDistance * 2);
}

int EconomyBalance::cityVaultUpgradeCost(const std::string& cityId, int currentLevel)
{
    if (currentLevel <= 0 || currentLevel >= 5)
    {
        return 0;
    }

    static const int baseCosts[] = {1000, 1500, 2250, 3500};
    return baseCosts[currentLevel - 1] * cityVaultMultiplier(cityId) / 100;
}

int EconomyBalance::cityVaultCapacityForLevel(int level)
{
    switch (std::max(0, std::min(5, level)))
    {
        case 1: return 250;
        case 2: return 300;
        case 3: return 350;
        case 4: return 400;
        case 5: return 500;
        default: return 0;
    }
}

int EconomyBalance::cityVaultExtraSlotsPerUpgrade()
{
    return 50;
}

int EconomyBalance::travelTimeUnitsForDistance(int distanceKm)
{
    if (distanceKm < 0) return 1;
    if (distanceKm <= 20) return 1;
    if (distanceKm <= 45) return 2;
    return 3;
}

int EconomyBalance::estimatedTravelCopperCost(int distanceKm)
{
    if (distanceKm <= 0) return 0;
    // EN: Not charged automatically yet; used as central reference for future caravans, tickets and services.
    // FR: Pas encore prélevé automatiquement ; référence pour futurs convois, billets et services.
    return 20 + distanceKm * 4;
}


int EconomyBalance::cityChangeTaxCopper(const std::string& destinationCityId, int distanceKm)
{
    return cityChangeTaxCopper(destinationCityId, distanceKm, false);
}

int EconomyBalance::cityChangeTaxCopper(const std::string& destinationCityId, int distanceKm, bool hasDestinationVault)
{
    if (distanceKm <= 0)
    {
        return 0;
    }

    int tax = estimatedTravelCopperCost(distanceKm) / 2;
    if (destinationCityId == "port_lanterne") tax += 35;
    else if (destinationCityId == "lys_de_givre") tax += 60;
    else if (destinationCityId == "rocheveille") tax += 25;
    else tax += 10;

    tax = std::max(5, tax);
    if (hasDestinationVault)
    {
        tax = std::max(3, (tax + 1) / 2);
    }
    return tax;
}

int EconomyBalance::routeRewardBudgetCopperForDistance(int distanceKm, int dangerLevel)
{
    const int safeDistance = std::max(1, distanceKm);
    const int safeDanger = std::max(0, dangerLevel);
    return 25 + safeDistance * 3 + safeDanger * 18;
}

int EconomyBalance::innCommonBedCostCopper(const std::string& cityId, int playerLevel)
{
    const CityShopContext context = CityProfileSystem::shopContextFor(cityId, ShopType::Lodging);
    return std::max(1, (96 + std::max(1, playerLevel) * 4) * (100 + context.buyPriceModifierPercent) / 100);
}

int EconomyBalance::innSafeRoomCostCopper(const std::string& cityId, int playerLevel)
{
    const CityShopContext context = CityProfileSystem::shopContextFor(cityId, ShopType::Lodging);
    return std::max(1, (185 + std::max(1, playerLevel) * 7) * (100 + context.buyPriceModifierPercent) / 100);
}

int EconomyBalance::innWarmMealCostCopper(const std::string& cityId, int playerLevel)
{
    const CityShopContext context = CityProfileSystem::shopContextFor(cityId, ShopType::Lodging);
    return std::max(12, (48 + std::max(1, playerLevel) * 2) * (100 + context.buyPriceModifierPercent) / 100);
}

int EconomyBalance::cityVaultMaterialTransferCostCopper(const std::string& fromCityId, const std::string& toCityId, int distanceKm, int quantity)
{
    const int safeDistance = std::max(1, distanceKm);
    const int safeQuantity = std::max(1, quantity);
    int cost = 45 + estimatedTravelCopperCost(safeDistance) / 2 + safeQuantity * 3;
    cost = cost * std::max(cityVaultMultiplier(fromCityId), cityVaultMultiplier(toCityId)) / 100;
    return std::max(35, cost);
}

int EconomyBalance::cityVaultCargoTransferCostCopper(const std::string& fromCityId, const std::string& toCityId, int distanceKm, int loadUnits)
{
    const int safeDistance = std::max(1, distanceKm);
    const int safeLoad = std::max(1, loadUnits);
    // Shared convoy pricing: route cost dominates, bulky cargo adds a readable supplement.
    int cost = 45 + estimatedTravelCopperCost(safeDistance) / 2 + safeLoad * 12;
    cost = cost * std::max(cityVaultMultiplier(fromCityId), cityVaultMultiplier(toCityId)) / 100;
    return std::max(35, cost);
}
