#include "quest/Quest.hpp"
#include "world/CityTradeFlowSystem.hpp"
#include "world/bounty/BountyTradeTransitSystem.hpp"

#include <cassert>
#include <iostream>
#include <string>

namespace
{
    Quest experiencedTarget()
    {
        Quest quest;
        quest.id = "bounty_trade_transit_test";
        quest.bountyMission = true;
        quest.accepted = true;
        quest.bountyTargetName = "Vela la Passeuse";
        quest.bountyTargetMobility = 82;
        quest.bountyTargetResources = 72;
        quest.bountyTargetKnowsHunt = true;
        quest.bountyDangerRating = 4;
        return quest;
    }
}

int main()
{
    // Rocheveille -> Port-Lanterne is an actual military trade relation. The bounty system must
    // only use it on days where the trade layer itself reports real activity/disruption.
    int usableDay = -1;
    CityTradeRouteActivity activity;
    for (int day = 0; day < 80; ++day)
    {
        const auto route = CityTradeFlowSystem::routeBetween(
            "rocheveille", "port_lanterne", day, ShopType::Weapon, "stable_market", "stable_market");
        if (route.supported && (route.departing || route.arriving || route.disrupted))
        {
            usableDay = day;
            activity = route;
            break;
        }
    }
    assert(usableDay >= 0);
    assert(activity.supported);
    assert(activity.sourceCityId == "rocheveille");
    assert(activity.destinationCityId == "port_lanterne");
    assert(activity.distance > 0);

    Quest quest = experiencedTarget();
    const auto opportunity = BountyTradeTransitSystem::assess(
        quest, "rocheveille", "port_lanterne", usableDay);
    assert(opportunity.available);
    assert(opportunity.goodsType != ShopType::Unknown);
    assert(!opportunity.goodsLabel.empty());
    assert(opportunity.destinationScoreBonus > 0);
    assert(opportunity.relocationChanceBonus > 0);
    assert(!opportunity.trailKind.empty());
    assert(opportunity.trailReliability >= 25);
    assert(opportunity.trailReliability <= 90);
    assert(!opportunity.summary.empty());

    // No self-route and no invented caravan on an invalid pair.
    const auto self = BountyTradeTransitSystem::assess(quest, "rocheveille", "rocheveille", usableDay);
    assert(!self.available);

    std::cout << "BountyTradeTransitSystemTest OK\n";
    return 0;
}
