// English: City travel and access rules for the future map and current terminal menus.
// Français : Règles de voyage et d'accès aux villes pour la future carte et les menus terminal actuels.
#ifndef INCLUDE_WORLD_CITYTRAVELRULES_HPP
#define INCLUDE_WORLD_CITYTRAVELRULES_HPP

#include "entity/Player.hpp"
#include "world/City.hpp"

#include <string>
#include <vector>

struct CityAccessReport
{
    bool allowed = false;
    std::vector<std::string> lines;
};

struct CityBuildingPreview
{
    std::string id;
    std::string name;
    std::string category;
    std::string contact;
    std::string detail;
    std::string pixelArtHint;
    bool unlocked = true;
};

class CityTravelRules
{
public:
    static CityAccessReport evaluateAccess(const Player& player, const City& city);
    static int getDistanceBetweenCities(const std::string& fromCityId, const std::string& toCityId);
    static int getTravelTaxCopper(const Player& player, const City& destination, int distanceKm);
    static int getRouteDiscoveryLimit(int distanceKm);
    static int getRouteDiscoveryCount(const Player& player, const std::string& fromCityId, const std::string& toCityId);
    static std::string buildNormalizedRouteKey(const std::string& fromCityId, const std::string& toCityId);
    static std::vector<std::string> buildTravelPreviewLines(const Player& player, const City& destination);
    static std::vector<CityBuildingPreview> getBuildingsForCity(const Player& player, const City& city);
    static std::vector<std::string> buildCityHubLines(const Player& player);
    static std::vector<std::string> buildLocalCityDifferentiationLines(const Player& player);
    static std::vector<std::string> buildRouteSystemLines(const Player& player, const City& destination);
    static bool isNightTravelClosed(const Player& player);
    static std::vector<std::string> buildNightTravelWarningLines(const Player& player);
    static int getRouteEventLimit(int distanceKm);
    static int getRouteEventCount(const Player& player, const std::string& fromCityId, const std::string& toCityId);
    static std::vector<std::string> buildExplorationMapLines(const Player& player);
    static std::vector<std::string> buildSchematicMapLines(const Player& player);
    static std::vector<std::string> buildFutureVisualPlanningLines();
};

#endif
