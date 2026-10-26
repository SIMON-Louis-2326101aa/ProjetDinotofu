#include "entity/Player.hpp"
#include "quest/Quest.hpp"
#include "world/bounty/BountyAftermathSystem.hpp"
#include "world/bounty/BountyRegionalConsequenceSystem.hpp"

#include <algorithm>
#include <cassert>

int main()
{
    Player player;
    player.setCurrentCityId("valebrume");

    Quest q;
    q.id = "bounty_aftermath:test";
    q.title = "Prime sur Marek";
    q.accepted = true;
    q.bountyMission = true;
    q.bountyTargetId = "marek_sans_lanterne";
    q.bountyTargetName = "Marek Sans-Lanterne";

    const int queued = BountyAftermathSystem::queueResolution(
        player, q, BountyResolutionKind::CapturedBySurrender);
    assert(queued == 3);

    auto sameDay = BountyAftermathSystem::pending(player);
    assert(sameDay.size() == 2); // issuer + organization; the close relation needs time to hear about it.
    assert(std::none_of(sameDay.begin(), sameDay.end(), [](const BountyAftermathEntry& entry) {
        return entry.actorType == BountyAftermathActorType::CloseRelation;
    }));

    const auto firstOptions = BountyAftermathSystem::responseOptions(sameDay.front());
    assert(!firstOptions.empty());
    const auto firstReply = BountyAftermathSystem::respond(
        player, sameDay.front().eventId, BountyAftermathResponse::Professional);
    assert(firstReply.resolved);
    assert(player.getCanonicalJournalCategoryTotal("bounty_aftermath_professional_responses") == 1);

    auto afterReply = BountyAftermathSystem::pending(player);
    assert(afterReply.size() == 1);

    player.advanceWorldDays(1);
    auto nextDay = BountyAftermathSystem::pending(player);
    assert(nextDay.size() == 2);
    const auto relative = std::find_if(nextDay.begin(), nextDay.end(), [](const BountyAftermathEntry& entry) {
        return entry.actorType == BountyAftermathActorType::CloseRelation;
    });
    assert(relative != nextDay.end());
    assert(relative->actorName == "Alis Sans-Lanterne");

    const auto relativeReply = BountyAftermathSystem::respond(
        player, relative->eventId, BountyAftermathResponse::Conciliatory);
    assert(relativeReply.resolved);
    assert(player.getCanonicalJournalCategoryTotal("bounty_close_relation_compassion") == 1);

    // A resolved follow-up is history, not a farmable notification.
    assert(BountyAftermathSystem::queueResolution(
        player, q, BountyResolutionKind::CapturedBySurrender) == 0);

    // A genuinely different final outcome is a distinct historical branch.
    assert(BountyAftermathSystem::queueResolution(
        player, q, BountyResolutionKind::KilledInCombat) == 3);
    return 0;
}
