#include "entity/Player.hpp"
#include "quest/QuestLog.hpp"
#include "world/CityTradeIncidentMissionSystem.hpp"

#include <cassert>
#include <iostream>

int main()
{
    Player player;
    player.setLoadedProgress(35, 0, 0);
    player.setCurrentCityId("port_lanterne");
    player.getQuestLog().ensureGuildBoardReady(player.getLevel(), player.getWorldDaysElapsed());

    CityTradeIncidentMissionOffer chosen;
    for (int day = 0; day < 900 && !chosen.valid; ++day)
    {
        const auto offers = CityTradeIncidentMissionSystem::offersFor(player);
        for (const auto& offer : offers)
        {
            if (offer.createsBountyLead && offer.severity >= 2)
            {
                chosen = offer;
                break;
            }
        }
        if (!chosen.valid) player.advanceWorldDays(1);
    }
    assert(chosen.valid);

    Quest quest = CityTradeIncidentMissionSystem::buildQuest(player, chosen);
    assert(quest.tradeIncidentMission);
    assert(quest.tradeIncidentStage == "preparation");
    assert(quest.target == 2);
    assert(quest.expiresAtDay > player.getWorldDaysElapsed());
    assert(player.getQuestLog().addQuest(quest));

    Quest* live = nullptr;
    for (Quest& q : player.getQuestLog().getQuests()) if (q.id == quest.id) live = &q;
    assert(live != nullptr);
    assert(CityTradeIncidentMissionSystem::prepare(player, *live));
    assert(live->tradeIncidentStage == "action");
    assert(live->progress == 1);

    // A cautious route can still fail, but repeated attempts eventually resolve a deterministic case.
    bool completed = false;
    for (int i = 0; i < 16 && !completed; ++i)
    {
        const auto result = CityTradeIncidentMissionSystem::resolve(player, *live, true);
        assert(result.attempted);
        completed = result.completed;
    }
    assert(completed);
    assert(live->completed);
    assert(live->tradeIncidentStage == "resolved");
    assert(live->tradeIncidentBountyPublished);

    bool foundBounty = false;
    for (const Quest& offer : player.getQuestLog().getGuildBoardOffers())
    {
        if (offer.id == "trade_incident_bounty:" + live->tradeIncidentId)
        {
            foundBounty = true;
            assert(offer.bountyMission);
            assert(offer.bountyTransitTrailAvailable);
        }
    }
    assert(foundBounty);

    // Publishing is idempotent.
    const std::size_t boardSize = player.getQuestLog().getGuildBoardOffers().size();
    assert(!CityTradeIncidentMissionSystem::publishBountyIfEligible(player, *live));
    assert(player.getQuestLog().getGuildBoardOffers().size() == boardSize);

    std::cout << "CityTradeIncidentMissionSystemTest OK\n";
    return 0;
}
