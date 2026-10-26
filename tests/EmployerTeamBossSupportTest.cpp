#include "boss/BossCatalog.hpp"
#include "class_system/ClassCatalog.hpp"
#include "combat/ally/RecruitedAllyBossSupport.hpp"
#include "combat/group/EmployerTeamRosterSystem.hpp"
#include "combat/group/EmployerTeamDisciplineSystem.hpp"
#include "combat/group/TeamEmploymentSystem.hpp"
#include "core/Random.hpp"
#include "entity/Player.hpp"

#include <cassert>
#include <vector>

int main()
{
    Player player("BossWorker", ClassCatalog::createClassByName("Guerrier"));
    assert(TeamEmploymentSystem::startExternalContract(
        player, "boss_contract", "Capitaine Ilya", "boss_company", "Compagnie du Front", "membre", 15, 7, 14));

    const EmployerTeamFormation formation = EmployerTeamRosterSystem::formationFor(player, true);
    assert(formation.activeMembers.size() == 2);
    assert(formation.activeMembers.front().leader);

    std::vector<RecruitedAllyCombatSupport> allies;
    for (const EmployerTeamMember& member : formation.activeMembers)
        allies.push_back(EmployerTeamRosterSystem::toCombatSupport(member));

    Boss boss = BossCatalog::createBoss(1);
    Random random(1234u);
    std::vector<std::string> lines;

    playRecruitedAlliesAgainstBoss(
        player, boss, allies, random, lines, RecruitedAllyBossTurnPhase::LeaderOnly);
    assert(allies.front().turnsTaken == 1);
    for (std::size_t i = 1; i < allies.size(); ++i) assert(allies[i].turnsTaken == 0);

    lines.clear();
    playRecruitedAlliesAgainstBoss(
        player, boss, allies, random, lines, RecruitedAllyBossTurnPhase::NonLeaderOnly);
    for (std::size_t i = 1; i < allies.size(); ++i) assert(allies[i].turnsTaken == 1);

    // Face à un boss, le chef employeur peut sacrifier son tour pour sauver un collègue blessé
    // sans attendre qu'il tombe. La réserve prend alors réellement la place.
    bool foundBossRotation = false;
    for (unsigned seed = 1; seed <= 10000 && !foundBossRotation; ++seed)
    {
        const EmployerTeamFormation rotationFormation = EmployerTeamRosterSystem::formationFor(player, true);
        if (rotationFormation.activeMembers.size() < 2 || rotationFormation.reserveMembers.empty()) break;
        std::vector<RecruitedAllyCombatSupport> rotationAllies;
        std::vector<RecruitedAllyCombatSupport> rotationReserve;
        for (const EmployerTeamMember& member : rotationFormation.activeMembers)
            rotationAllies.push_back(EmployerTeamRosterSystem::toCombatSupport(member));
        for (const EmployerTeamMember& member : rotationFormation.reserveMembers)
            rotationReserve.push_back(EmployerTeamRosterSystem::toCombatSupport(member));
        rotationAllies[1].currentHp = std::max(1, rotationAllies[1].maxHp / 10);
        const std::string woundedName = rotationAllies[1].name;
        const std::string reserveName = rotationReserve.front().name;
        Random rotationRandom(seed);
        std::vector<std::string> rotationLines;
        if (tryEmployerBossLeaderRotation(player, rotationAllies, rotationReserve, rotationRandom, rotationLines))
        {
            foundBossRotation = true;
            assert(rotationAllies.front().teamLeader);
            assert(rotationAllies.front().turnsTaken == 1);
            bool reserveEntered = false;
            for (const auto& member : rotationAllies)
            {
                if (member.name == reserveName)
                {
                    reserveEntered = true;
                    assert(member.enteredFromReserveThisPhase);
                }
            }
            assert(reserveEntered);
            bool woundedRetreated = false;
            for (const auto& member : rotationReserve) if (member.name == woundedName) woundedRetreated = true;
            assert(woundedRetreated);

            // Une entrée depuis la réserve ne donne jamais un tour gratuit dans la phase en cours.
            const int incomingTurnsBefore = [&]() {
                for (const auto& member : rotationAllies) if (member.name == reserveName) return member.turnsTaken;
                return -1;
            }();
            playRecruitedAlliesAgainstBoss(
                player, boss, rotationAllies, rotationRandom, rotationLines, RecruitedAllyBossTurnPhase::NonLeaderOnly);
            for (const auto& member : rotationAllies)
            {
                if (member.name == reserveName) assert(member.turnsTaken == incomingTurnsBefore);
            }
        }
    }
    assert(foundBossRotation);


    // Un salarié sanctionné peut commencer un boss de service sur le banc. Le chef peut le rappeler
    // volontairement sans attendre un KO, en sacrifiant son propre tour et en libérant une place.
    Player benched("BenchedBossWorker", ClassCatalog::createClassByName("Guerrier"));
    assert(TeamEmploymentSystem::startExternalContract(
        benched, "boss_bench_contract", "Capitaine Ilya", "boss_company", "Compagnie du Front", "membre", 15, 7, 14));
    EmployerTeamDisciplineSystem::recordMissionFailed(benched, "bench_fail_1", "ordre_non_accompli");
    EmployerTeamDisciplineSystem::recordMissionFailed(benched, "bench_fail_2", "ordre_non_accompli");
    assert(EmployerTeamDisciplineSystem::playerStartsInReserve(benched));
    const int reserveBefore = EmployerTeamDisciplineSystem::reserveBattlesRemaining(benched);

    bool foundPlayerRecall = false;
    for (unsigned seed = 1; seed <= 10000 && !foundPlayerRecall; ++seed)
    {
        const EmployerTeamFormation benchFormation = EmployerTeamRosterSystem::formationFor(benched, true);
        assert(benchFormation.playerInReserve);
        assert(benchFormation.activeMembers.size() == 3);
        std::vector<RecruitedAllyCombatSupport> benchAllies;
        std::vector<RecruitedAllyCombatSupport> benchReserve;
        for (const EmployerTeamMember& member : benchFormation.activeMembers)
            benchAllies.push_back(EmployerTeamRosterSystem::toCombatSupport(member));
        for (const EmployerTeamMember& member : benchFormation.reserveMembers)
            benchReserve.push_back(EmployerTeamRosterSystem::toCombatSupport(member));

        const std::size_t reserveSizeBefore = benchReserve.size();
        Random recallRandom(seed);
        std::vector<std::string> recallLines;
        if (tryEmployerBossLeaderRecallPlayer(
                benched, benchAllies, benchReserve, recallRandom, 2, recallLines))
        {
            foundPlayerRecall = true;
            assert(benchAllies.size() == 2);
            assert(benchReserve.size() == reserveSizeBefore + 1);
            assert(!recallLines.empty());
            bool leaderSpentTurn = false;
            for (const auto& member : benchAllies)
            {
                if (member.teamLeader)
                {
                    leaderSpentTurn = true;
                    assert(member.turnsTaken == 1);
                }
            }
            assert(leaderSpentTurn);
        }
    }
    assert(foundPlayerRecall);
    // Le helper de rotation ne purge pas lui-même la sanction : c'est le combat de service qui le fait.
    assert(EmployerTeamDisciplineSystem::reserveBattlesRemaining(benched) == reserveBefore);

    allies.front().currentHp = std::max(1, allies.front().maxHp / 2);
    persistBossRecruitedAllyVitals(player, allies, "boss_employeur_test");
    const auto persisted = EmployerTeamRosterSystem::rosterFor(player);
    bool foundLeader = false;
    for (const EmployerTeamMember& member : persisted)
    {
        if (!member.leader) continue;
        foundLeader = true;
        assert(member.currentHp == allies.front().currentHp);
    }
    assert(foundLeader);
    return 0;
}
