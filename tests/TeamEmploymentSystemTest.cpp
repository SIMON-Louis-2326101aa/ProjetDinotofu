#include "combat/group/TeamEmploymentSystem.hpp"
#include "combat/group/TeamLeadershipSystem.hpp"
#include "class_system/ClassCatalog.hpp"
#include "entity/Player.hpp"

#include <cassert>

int main()
{
    Player player("Trexof", ClassCatalog::createClassByName("Guerrier"));
    assert(!TeamEmploymentSystem::hasActiveContract(player));

    const long long moneyBefore = player.getInventory().getEconomyUnits();
    assert(TeamEmploymentSystem::startExternalContract(
        player,
        "garde_est_patrouille_01",
        "Capitaine Maelis",
        "garde_de_lest",
        "Garde de l'Est",
        "éclaireur sous contrat",
        12,
        7,
        21
    ));
    assert(!TeamEmploymentSystem::startExternalContract(player, "double", "Chef", "org", "Org", "membre", 1));

    TeamEmploymentContract contract = TeamEmploymentSystem::contractFor(player);
    assert(contract.active);
    assert(contract.contractId == "garde_est_patrouille_01");
    assert(contract.leaderName == "Capitaine Maelis");
    assert(contract.organizationId == "garde_de_lest");
    assert(contract.salaryEconomyUnits == 12);
    assert(contract.payIntervalDays == 7);
    assert(contract.plannedEndDay == contract.startDay + 21);

    TeamLeadershipContext leadership = TeamLeadershipSystem::contextFor(player);
    assert(leadership.leaderKind == TeamLeaderKind::ExternalNpc);
    assert(!leadership.playerIsLeader);
    assert(!leadership.canIssueTacticalOrders);
    assert(!leadership.canArrangePassageOrder);
    assert(!leadership.playerPaysTeamCosts);

    assert(TeamEmploymentSystem::dueSalaryPeriods(player) == 0);
    player.advanceWorldDays(6);
    assert(TeamEmploymentSystem::dueSalaryPeriods(player) == 0);
    player.advanceWorldDays(1);
    assert(TeamEmploymentSystem::dueSalaryPeriods(player) == 1);
    assert(TeamEmploymentSystem::dueSalaryEconomyUnits(player) == 12);

    TeamSalarySettlement firstPay = TeamEmploymentSystem::collectDueSalary(player);
    assert(firstPay.paid);
    assert(firstPay.periodsPaid == 1);
    assert(firstPay.economyUnitsPaid == 12);
    assert(player.getInventory().getEconomyUnits() >= moneyBefore + 12);
    assert(TeamEmploymentSystem::dueSalaryPeriods(player) == 0);

    TeamEmploymentSystem::recordEmployerMissionCompleted(player, "mission_1", "Patrouille du pont");
    TeamEmploymentSystem::recordEmployerMissionFailed(player, "mission_2", "Escorte interrompue");
    assert(player.getCanonicalJournalCategoryTotal("team_employment_missions_completed") == 1);
    assert(player.getCanonicalJournalCategoryTotal("team_employment_missions_failed") == 1);

    player.advanceWorldDays(14);
    assert(TeamEmploymentSystem::dueSalaryPeriods(player) == 2);
    assert(TeamEmploymentSystem::finalizeIfExpired(player));
    assert(!TeamEmploymentSystem::hasActiveContract(player));
    assert(TeamLeadershipSystem::contextFor(player).playerIsLeader);
    assert(player.getCanonicalJournalCategoryTotal("team_employment_payments") >= 36);

    Player resigning("Crystal", ClassCatalog::createClassByName("Mage"));
    assert(TeamEmploymentSystem::startExternalContract(
        resigning, "short", "Lieutenant A", "escouade_a", "Escouade A", "mage de soutien", 9, 3, -1));
    resigning.advanceWorldDays(3);
    const long long resignMoneyBefore = resigning.getInventory().getEconomyUnits();
    assert(TeamEmploymentSystem::resign(resigning, "choix_personnel"));
    assert(resigning.getInventory().getEconomyUnits() >= resignMoneyBefore + 9);
    assert(!TeamEmploymentSystem::hasActiveContract(resigning));
    assert(TeamLeadershipSystem::contextFor(resigning).playerIsLeader);

    return 0;
}
