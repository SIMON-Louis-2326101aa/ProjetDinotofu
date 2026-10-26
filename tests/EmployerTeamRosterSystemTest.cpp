#include "combat/group/EmployerTeamRosterSystem.hpp"
#include "class_system/ClassCatalog.hpp"
#include "combat/group/TeamEmploymentSystem.hpp"
#include "entity/Player.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

int main()
{
    Player player("RosterTester", ClassCatalog::createClassByName("Guerrier"));

    assert(TeamEmploymentSystem::startExternalContract(
        player,
        "contract_roster_test",
        "Capitaine Maelis",
        "test_company",
        "Compagnie du Test",
        "membre de terrain",
        12,
        7,
        14));

    // startExternalContract crée maintenant automatiquement le roster.
    const auto roster = EmployerTeamRosterSystem::rosterFor(player);
    assert(roster.size() >= 3);
    assert(roster.size() <= 5);
    assert(roster.front().leader);
    assert(roster.front().name == "Capitaine Maelis");

    // Avec le joueur conscient : joueur + 2 PNJ max. Le chef est toujours le premier PNJ à jouer.
    const EmployerTeamFormation withPlayer = EmployerTeamRosterSystem::formationFor(player, true);
    assert(!withPlayer.playerInReserve);
    assert(withPlayer.activeMembers.size() == 2);
    assert(withPlayer.activeMembers.front().leader);
    assert(withPlayer.activeMembers.front().name == "Capitaine Maelis");
    assert(withPlayer.activeMembers.size() + withPlayer.reserveMembers.size() + withPlayer.unavailableMembers.size() == roster.size());
    const auto turnOrder = EmployerTeamRosterSystem::openingTurnOrder(player, true);
    assert(turnOrder.size() == 3);
    assert(turnOrder[0] == "Capitaine Maelis");
    assert(turnOrder[1] == player.getName());

    // Après chute du joueur : sa place est libérée, donc jusqu'à 3 PNJ actifs, chef toujours premier.
    const EmployerTeamFormation withoutPlayer = EmployerTeamRosterSystem::formationFor(player, false);
    assert(withoutPlayer.activeMembers.size() == 3);
    assert(withoutPlayer.activeMembers.front().leader);
    const auto orderWithoutPlayer = EmployerTeamRosterSystem::openingTurnOrder(player, false);
    assert(orderWithoutPlayer.size() == 3);
    assert(orderWithoutPlayer.front() == "Capitaine Maelis");

    // Un collègue reste un vrai individu : PV/indisponibilité persistent et le font sortir de la ligne active.
    const std::string secondId = roster[1].memberId;
    assert(EmployerTeamRosterSystem::setMemberVitals(player, secondId, 0, false, "ko_infirmerie"));
    const EmployerTeamFormation afterKo = EmployerTeamRosterSystem::formationFor(player, true);
    bool foundUnavailable = false;
    for (const auto& member : afterKo.unavailableMembers)
    {
        if (member.memberId == secondId) foundUnavailable = true;
    }
    assert(foundUnavailable);

    // Le chef indisponible ne reçoit évidemment pas un tour fantôme ; la règle "chef premier" ne ressuscite personne.
    assert(EmployerTeamRosterSystem::setMemberVitals(player, "leader", 0, false, "ko_infirmerie"));
    const EmployerTeamFormation leaderDown = EmployerTeamRosterSystem::formationFor(player, true);
    assert(!leaderDown.activeMembers.empty());
    assert(!leaderDown.activeMembers.front().leader);

    // Progression indépendante du joueur et aucun prélèvement de part sur son butin.
    const auto support = EmployerTeamRosterSystem::toCombatSupport(roster[1]);
    assert(support.requestedShare == 0);
    assert(support.normalizedShare == 0);
    assert(support.teamOrigin == AllyTeamOrigin::EmployerTeam);
    assert(!support.persistentMemberId.empty());
    assert(EmployerTeamRosterSystem::recordMemberCombatProgress(player, roster[1].memberId, 12));

    const int readyDay = player.getWorldDaysElapsed() + 2;
    assert(EmployerTeamRosterSystem::setMemberVitals(
        player, roster[2].memberId, 0, false, "ko_escouade | ready_day=" + std::to_string(readyDay)));
    player.advanceWorldDays(2);
    const auto recoveredRoster = EmployerTeamRosterSystem::rosterFor(player);
    auto recovered = std::find_if(recoveredRoster.begin(), recoveredRoster.end(), [&](const EmployerTeamMember& member) {
        return member.memberId == roster[2].memberId;
    });
    assert(recovered != recoveredRoster.end());
    assert(recovered->available);
    assert(recovered->currentHp > 0);

    std::cout << "EmployerTeamRosterSystemTest OK\n";
    return 0;
}
