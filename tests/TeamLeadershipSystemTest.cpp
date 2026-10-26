#include "combat/group/TeamLeadershipSystem.hpp"
#include "class_system/ClassCatalog.hpp"
#include "entity/Player.hpp"

#include <cassert>

int main()
{
    Player player("Trexof", ClassCatalog::createClassByName("Guerrier"));

    auto personal = TeamLeadershipSystem::contextFor(player);
    assert(personal.leaderKind == TeamLeaderKind::PlayerCharacter);
    assert(personal.playerIsLeader);
    assert(personal.canIssueTacticalOrders);
    assert(personal.canArrangePassageOrder);
    assert(!personal.playerIsPaidMember);
    assert(personal.playerPaysTeamCosts);
    assert(TeamLeadershipSystem::leaderDisplayName(player) == "Trexof");

    TeamLeadershipSystem::setExternalNpcTeam(
        player,
        "Capitaine Maelis",
        "garde_de_lest",
        "Garde de l'Est",
        480
    );
    auto employed = TeamLeadershipSystem::contextFor(player);
    assert(employed.leaderKind == TeamLeaderKind::ExternalNpc);
    assert(!employed.playerIsLeader);
    assert(!employed.canIssueTacticalOrders);
    assert(!employed.canArrangePassageOrder);
    assert(employed.playerIsPaidMember);
    assert(!employed.playerPaysTeamCosts);
    assert(employed.leaderName == "Capitaine Maelis");
    assert(employed.organizationId == "garde_de_lest");
    assert(employed.organizationName == "Garde de l'Est");
    assert(employed.salaryEconomyUnits == 480);

    TeamLeadershipSystem::setOtherPlayerLedTeam(player, "Crystal", 0);
    auto otherPlayer = TeamLeadershipSystem::contextFor(player);
    assert(otherPlayer.leaderKind == TeamLeaderKind::OtherPlayer);
    assert(!otherPlayer.playerIsLeader);
    assert(otherPlayer.leaderName == "Crystal");
    assert(!otherPlayer.playerIsPaidMember);
    assert(!otherPlayer.playerPaysTeamCosts);

    TeamLeadershipSystem::setPersonalTeam(player);
    personal = TeamLeadershipSystem::contextFor(player);
    assert(personal.playerIsLeader);
    assert(personal.canIssueTacticalOrders);
    assert(personal.canArrangePassageOrder);
    assert(personal.salaryEconomyUnits == 0);
    assert(personal.organizationId.empty());

    return 0;
}
