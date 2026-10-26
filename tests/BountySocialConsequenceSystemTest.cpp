#include "entity/Player.hpp"
#include "quest/Quest.hpp"
#include "world/bounty/BountyRegionalConsequenceSystem.hpp"

#include <cassert>

int main()
{
    Player player;
    player.setCurrentCityId("valebrume");
    PlayerCrimeContactState& known = player.getOrCreateCrimeContactState(
        "valebrume_receleur_brocanteur",
        "Serrin le brocanteur");
    const int pressureBefore = known.pressure;

    Quest q;
    q.id = "bounty_social:test";
    q.title = "Prime test";
    q.accepted = true;
    q.bountyMission = true;
    q.bountyTargetId = "marek_sans_lanterne";
    q.bountyTargetName = "Marek Sans-Lanterne";
    q.bountyTargetNetworkId = "marche_gris";
    q.bountyIssuerId = "orren";
    q.bountyIssuerName = "Orren";
    q.bountyIssuerOrganizationId = "poste_orren";
    q.bountyIssuerOrganizationName = "Poste d'Orren";
    q.bountyIssuerPrefersLiveCapture = true;
    q.bountyCloseRelationId = "alis_sans_lanterne";
    q.bountyCloseRelationName = "Alis Sans-Lanterne";
    q.bountyCloseRelationType = "sœur";

    auto result = BountyRegionalConsequenceSystem::applyResolution(
        player,
        q,
        BountyResolutionKind::CapturedBySurrender);
    assert(result.applied);
    assert(player.getCanonicalJournalCategoryTotal("bounty_issuer_positive_reactions") == 1);
    assert(player.getCanonicalJournalCategoryTotal("bounty_organization_positive_reactions") == 1);
    assert(player.getCanonicalJournalCategoryTotal("bounty_close_relation_capture_reactions") == 1);
    assert(player.getCanonicalJournalCategoryTotal("bounty_named_network_reactions") >= 1);
    const PlayerCrimeContactState* after = player.findCrimeContactState("valebrume_receleur_brocanteur");
    assert(after != nullptr);
    assert(after->pressure > pressureBefore);

    Quest lethal = q;
    lethal.id = "bounty_social:lethal";
    lethal.bountyRegionalConsequencesApplied = false;
    lethal.bountyResolution.clear();
    auto lethalResult = BountyRegionalConsequenceSystem::applyResolution(
        player,
        lethal,
        BountyResolutionKind::KilledInCombat);
    assert(lethalResult.applied);
    assert(player.getCanonicalJournalCategoryTotal("bounty_issuer_mixed_reactions") == 1);
    assert(player.getCanonicalJournalCategoryTotal("bounty_close_relation_death_reactions") == 1);
    return 0;
}
