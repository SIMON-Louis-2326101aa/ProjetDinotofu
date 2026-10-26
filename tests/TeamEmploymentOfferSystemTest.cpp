#include "combat/group/TeamEmploymentOfferSystem.hpp"
#include "combat/group/TeamEmploymentSystem.hpp"
#include "combat/group/TeamEmploymentMissionSystem.hpp"
#include "combat/group/TeamLeadershipSystem.hpp"
#include "class_system/ClassCatalog.hpp"
#include "entity/Player.hpp"

#include <cassert>

int main()
{
    Player player("Trexof", ClassCatalog::createClassByName("Guerrier"));
    TeamEmploymentOffer offer = TeamEmploymentOfferSystem::localOfferFor(player);
    assert(offer.available);
    assert(!offer.contractId.empty());
    assert(!offer.leaderName.empty());
    assert(!offer.organizationId.empty());
    assert(!offer.organizationName.empty());
    assert(offer.salaryEconomyUnits >= 4 && offer.salaryEconomyUnits <= 25);
    assert(offer.payIntervalDays == 7);
    assert(offer.durationDays == 14);

    assert(TeamEmploymentOfferSystem::acceptLocalOffer(player));
    assert(TeamEmploymentSystem::hasActiveContract(player));
    assert(TeamEmploymentMissionSystem::hasOpenEmployerMission(player));
    const TeamLeadershipContext leadership = TeamLeadershipSystem::contextFor(player);
    assert(leadership.leaderKind == TeamLeaderKind::ExternalNpc);
    assert(!leadership.playerIsLeader);
    assert(!TeamEmploymentOfferSystem::localOfferFor(player).available);

    return 0;
}
