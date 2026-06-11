// EN: Central economy balance constants for storage, travel and future shared pricing.
// FR: Constantes centrales d'économie pour coffres, voyages et futurs prix communs.
#ifndef INCLUDE_ECONOMY_ECONOMYBALANCE_HPP
#define INCLUDE_ECONOMY_ECONOMYBALANCE_HPP

#include <string>

class EconomyBalance
{
public:
    static int cityVaultPurchaseCost(const std::string& cityId);
    static int cityVaultUpgradeCost(const std::string& cityId, int currentLevel);
    static int cityVaultCapacityForLevel(int level);
    static int cityVaultExtraSlotsPerUpgrade();

    static int travelTimeUnitsForDistance(int distanceKm);
    static int estimatedTravelCopperCost(int distanceKm);
    static int cityChangeTaxCopper(const std::string& destinationCityId, int distanceKm);
    static int cityChangeTaxCopper(const std::string& destinationCityId, int distanceKm, bool hasDestinationVault);
    static int routeRewardBudgetForDistance(int distanceKm, int dangerLevel);

    static int innCommonBedCost(const std::string& cityId, int playerLevel);
    static int innSafeRoomCost(const std::string& cityId, int playerLevel);
    static int innWarmMealCost(const std::string& cityId, int playerLevel);
    static int cityVaultMaterialTransferCost(const std::string& fromCityId, const std::string& toCityId, int distanceKm, int quantity);
};

#endif
