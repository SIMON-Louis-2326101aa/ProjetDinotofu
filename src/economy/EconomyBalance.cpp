// EN: Centralized economy numbers keep related systems from drifting apart.
// FR: Les nombres économiques centralisés évitent que les systèmes se contredisent.
#include "economy/EconomyBalance.hpp"

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

int EconomyBalance::cityVaultPurchaseCost(const std::string& cityId)
{
    return 350 * cityVaultMultiplier(cityId) / 100;
}

int EconomyBalance::cityVaultUpgradeCost(const std::string& cityId, int currentLevel)
{
    if (currentLevel <= 0 || currentLevel >= 5)
    {
        return 0;
    }

    int cost = 650 * cityVaultMultiplier(cityId) / 100;
    for (int level = 1; level < currentLevel; ++level)
    {
        cost *= 2;
    }
    return cost;
}

int EconomyBalance::cityVaultCapacityForLevel(int level)
{
    if (level <= 0)
    {
        return 0;
    }
    const int clampedLevel = std::max(1, std::min(5, level));
    return 12 + (clampedLevel - 1) * cityVaultExtraSlotsPerUpgrade();
}

int EconomyBalance::cityVaultExtraSlotsPerUpgrade()
{
    return 8;
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

int EconomyBalance::routeRewardBudgetForDistance(int distanceKm, int dangerLevel)
{
    const int safeDistance = std::max(1, distanceKm);
    const int safeDanger = std::max(0, dangerLevel);
    return 25 + safeDistance * 3 + safeDanger * 18;
}

int EconomyBalance::innCommonBedCost(const std::string& cityId, int playerLevel)
{
    return (55 + std::max(1, playerLevel) * 3) * cityVaultMultiplier(cityId) / 100;
}

int EconomyBalance::innSafeRoomCost(const std::string& cityId, int playerLevel)
{
    return (120 + std::max(1, playerLevel) * 6) * cityVaultMultiplier(cityId) / 100;
}

int EconomyBalance::innWarmMealCost(const std::string& cityId, int playerLevel)
{
    return std::max(8, (24 + std::max(1, playerLevel)) * cityVaultMultiplier(cityId) / 100);
}

int EconomyBalance::cityVaultMaterialTransferCost(const std::string& fromCityId, const std::string& toCityId, int distanceKm, int quantity)
{
    const int safeDistance = std::max(1, distanceKm);
    const int safeQuantity = std::max(1, quantity);
    int cost = 45 + estimatedTravelCopperCost(safeDistance) / 2 + safeQuantity * 3;
    cost = cost * std::max(cityVaultMultiplier(fromCityId), cityVaultMultiplier(toCityId)) / 100;
    return std::max(35, cost);
}
