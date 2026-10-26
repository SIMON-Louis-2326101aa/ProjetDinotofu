#pragma once

#include "economy/shop/ShopType.hpp"
#include "quest/Quest.hpp"

#include <string>
#include <vector>

class Player;

struct CityTradeIncidentMissionOffer
{
    bool valid = false;
    std::string incidentId;
    std::string incidentKind;
    std::string headline;
    std::string summary;
    std::string sourceCityId;
    std::string sourceCityName;
    std::string destinationCityId;
    std::string destinationCityName;
    ShopType goodsType = ShopType::Unknown;
    int severity = 0;
    bool createsBountyLead = false;
    bool escortOpportunity = false;
};

struct CityTradeIncidentMissionResult
{
    bool attempted = false;
    bool success = false;
    bool completed = false;
    bool bountyPublished = false;
    int chancePercent = 0;
    int timeUnits = 0;
    std::vector<std::string> lines;
};

namespace CityTradeIncidentMissionSystem
{
    std::vector<CityTradeIncidentMissionOffer> offersFor(const Player& player);
    Quest buildQuest(const Player& player, const CityTradeIncidentMissionOffer& offer);
    bool prepare(Player& player, Quest& quest);
    CityTradeIncidentMissionResult resolve(Player& player, Quest& quest, bool cautiousApproach);
    bool publishBountyIfEligible(Player& player, Quest& quest);
    bool isActiveIncidentQuest(const Quest& quest);
}
