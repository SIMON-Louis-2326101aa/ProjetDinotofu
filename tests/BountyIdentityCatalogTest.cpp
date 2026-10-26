#include "quest/Quest.hpp"
#include "world/bounty/BountyIdentityCatalog.hpp"
#include "world/bounty/BountyQuestSystem.hpp"

#include <cassert>

int main()
{
    const auto& catalog = BountyIdentityCatalog::getCatalog();
    assert(catalog.size() >= 8);
    assert(BountyIdentityCatalog::findById("marek_sans_lanterne") != nullptr);
    assert(BountyIdentityCatalog::findByName("Neris le Fuyard") != nullptr);

    Quest q;
    q.id = "bounty_identity:test";
    q.title = "Prime ciblée";
    q.rewardGold = 100;
    q.accepted = true;
    BountyQuestSystem::initializeBounty(q, 25);

    assert(q.bountyMission);
    assert(!q.bountyTargetId.empty());
    assert(!q.bountyTargetName.empty());
    assert(!q.bountyIssuerId.empty());
    assert(!q.bountyIssuerOrganizationId.empty());
    assert(!q.bountyCloseRelationId.empty());
    assert(BountyIdentityCatalog::findById(q.bountyTargetId) != nullptr);

    Quest legacy;
    legacy.id = "legacy_named_bounty";
    legacy.title = "Ancienne prime";
    legacy.rewardGold = 80;
    legacy.accepted = true;
    legacy.bountyTargetName = "Marek Sans-Lanterne";
    legacy.bountyTargetTemperament = "prudent";
    BountyQuestSystem::initializeBounty(legacy, 10);
    assert(legacy.bountyTargetId == "marek_sans_lanterne");
    assert(legacy.bountyIssuerName == "Orren");
    assert(legacy.bountyTargetNetworkId == "marche_gris");
    return 0;
}
