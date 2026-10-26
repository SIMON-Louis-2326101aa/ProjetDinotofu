#include "combat/ally/RecruitedAllyAftermathSystem.hpp"
#include "combat/group/EmployerTeamRosterSystem.hpp"
#include "combat/group/TeamEmploymentSystem.hpp"
#include "class_system/ClassCatalog.hpp"
#include "core/Random.hpp"
#include "entity/Player.hpp"

#include <cassert>
#include <vector>

int main()
{
    Player player("Worker", ClassCatalog::createClassByName("Guerrier"));
    assert(TeamEmploymentSystem::startExternalContract(
        player,
        "contract_aftermath_test",
        "Capitaine Rhea",
        "company_after_test",
        "Compagnie des Veilleurs",
        "membre de terrain",
        10,
        7,
        14));

    const auto roster = EmployerTeamRosterSystem::rosterFor(player);
    assert(roster.size() >= 3);

    std::vector<RecruitedAllyCombatSupport> survivors;
    for (const EmployerTeamMember& member : EmployerTeamRosterSystem::formationFor(player, true).activeMembers)
    {
        survivors.push_back(EmployerTeamRosterSystem::toCombatSupport(member));
    }
    assert(!survivors.empty());
    assert(survivors.front().teamLeader);

    const int reportChance = RecruitedAllyAftermathSystem::reportChancePercent(player, survivors.front());
    assert(reportChance >= 45 && reportChance <= 98);

    Random random(42u);
    const RecruitedAllyAftermathDecision report = RecruitedAllyAftermathSystem::decide(
        player, survivors, random, "employer_report_test");
    assert(!report.memberName.empty());
    assert(report.teamOrigin == AllyTeamOrigin::EmployerTeam);
    assert(!report.persistentMemberId.empty());

    const long long beforeDelivery = player.getInventory().getEconomyUnits();
    RecruitedAllyAftermathDecision honest = report;
    honest.stealsPlayerShare = false;
    assert(RecruitedAllyAftermathSystem::deliverPlayerEconomyShare(player, 23, honest, "employer_delivery_test") == 23);
    assert(player.getInventory().getEconomyUnits() == beforeDelivery + 23);
    assert(player.getCanonicalJournalCategoryTotal("team_employer_share_deliveries") >= 23);

    RecruitedAllyAftermathDecision betrayal = report;
    betrayal.stealsPlayerShare = true;
    const long long beforeTheft = player.getInventory().getEconomyUnits();
    assert(RecruitedAllyAftermathSystem::deliverPlayerEconomyShare(player, 31, betrayal, "employer_theft_test") == 0);
    assert(player.getInventory().getEconomyUnits() == beforeTheft);
    assert(player.getCanonicalJournalCategoryTotal("team_employer_share_thefts") >= 31);

    const auto after = EmployerTeamRosterSystem::rosterFor(player);
    bool traitorStillAvailable = false;
    for (const EmployerTeamMember& member : after)
    {
        if (member.memberId == betrayal.persistentMemberId)
        {
            traitorStillAvailable = member.available;
        }
    }
    assert(!traitorStillAvailable);
    return 0;
}
