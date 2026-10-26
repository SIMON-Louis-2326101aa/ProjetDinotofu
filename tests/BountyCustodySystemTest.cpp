#include "entity/Player.hpp"
#include "quest/Quest.hpp"
#include "world/bounty/BountyCustodySystem.hpp"
#include "world/bounty/BountyRegionalConsequenceSystem.hpp"

#include <cassert>
#include <string>

int main()
{
    Player player;
    player.setCurrentCityId("valebrume");
    player.getOrCreateCrimeContactState("valebrume_receleur_brocanteur", "Serrin le brocanteur");

    Quest q;
    q.id = "bounty_custody:test";
    q.title = "Prime sur Marek";
    q.accepted = true;
    q.bountyMission = true;
    q.bountyTargetId = "marek_sans_lanterne";
    q.bountyTargetName = "Marek Sans-Lanterne";

    assert(BountyCustodySystem::queueCapture(player, q, BountyResolutionKind::CapturedBySurrender));
    assert(!BountyCustodySystem::queueCapture(player, q, BountyResolutionKind::CapturedBySurrender));
    assert(BountyCustodySystem::pendingCount(player) == 1);

    const BountyCustodyEntry entry = BountyCustodySystem::pending(player).front();
    const auto options = BountyCustodySystem::options(entry);
    assert(options.size() == 3);

    const BountyCustodyResult result = BountyCustodySystem::resolve(
        player, entry.eventId, BountyCustodyApproach::AskAboutNetwork);
    assert(result.resolved);
    assert(result.testimonyRecorded);
    assert(result.knownNetworkContactsAffected == 1);
    assert(BountyCustodySystem::pendingCount(player) == 0);
    assert(player.getCanonicalJournalCategoryTotal("bounty_custody_network_leads") >= 1);

    // A lethal resolution never creates a custody interview.
    Quest lethal = q;
    lethal.id = "bounty_custody:lethal";
    lethal.bountyTargetId = "darel_coupe_corde";
    lethal.bountyTargetName = "Darel Coupe-Corde";
    assert(!BountyCustodySystem::queueCapture(player, lethal, BountyResolutionKind::KilledInCombat));
    return 0;
}
