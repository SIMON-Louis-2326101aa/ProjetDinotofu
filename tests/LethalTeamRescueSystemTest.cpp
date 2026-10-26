#include "combat/death/LethalTeamRescueSystem.hpp"
#include "class_system/ClassCatalog.hpp"
#include "core/Random.hpp"
#include "entity/Player.hpp"

#include <cassert>
#include <vector>

namespace
{
    RecruitedAllyCombatSupport ally(const std::string& name, const std::string& job, int rank, int kit, int hp = 100)
    {
        RecruitedAllyCombatSupport result;
        result.name = name;
        result.job = job;
        result.trait = "calme";
        result.motivation = "proteger le groupe";
        result.currentRankIndex = rank;
        result.estimatedLevel = 20 + rank * 10;
        result.recruitedLevel = 20;
        result.supportKitQuality = kit;
        result.maxHp = 100;
        result.currentHp = hp;
        return result;
    }

    unsigned int seedForSuccess(int chance)
    {
        for (unsigned int seed = 1; seed < 10000; ++seed)
        {
            Random probe(seed);
            if (probe.between(1, 100) <= chance) return seed;
        }
        return 1;
    }

    unsigned int seedForFailure(int chance)
    {
        for (unsigned int seed = 1; seed < 10000; ++seed)
        {
            Random probe(seed);
            if (probe.between(1, 100) > chance) return seed;
        }
        return 1;
    }
}

int main()
{
    const auto emptyAssessment = LethalTeamRescueSystem::assessRecruitedTeam({}, {});
    assert(!emptyAssessment.teamPresent);
    assert(emptyAssessment.survivalChancePercent == 0);

    std::vector<RecruitedAllyCombatSupport> warriors = {ally("Boros", "Vieux guerrier", 3, 1)};
    const auto warriorAssessment = LethalTeamRescueSystem::assessRecruitedTeam(warriors, {});
    assert(warriorAssessment.teamPresent);
    assert(!warriorAssessment.hasCapableRescuer);
    assert(warriorAssessment.survivalChancePercent == 15);
    assert(warriorAssessment.revivedHealthPercent == 5);

    // Un excellent kit ne transforme pas un guerrier sans formation médicale en réanimateur.
    std::vector<RecruitedAllyCombatSupport> equippedWarriors = {ally("Doran", "Vieux guerrier", 6, 10)};
    const auto equippedWarriorAssessment = LethalTeamRescueSystem::assessRecruitedTeam(equippedWarriors, {});
    assert(equippedWarriorAssessment.teamPresent);
    assert(!equippedWarriorAssessment.hasCapableRescuer);
    assert(equippedWarriorAssessment.survivalChancePercent == 15);

    std::vector<RecruitedAllyCombatSupport> deadHealerWithWarrior = {
        ally("Aline", "Soigneur", 6, 10, 0),
        ally("Boros", "Vieux guerrier", 3, 1, 100)
    };
    const auto deadHealerAssessment = LethalTeamRescueSystem::assessRecruitedTeam(deadHealerWithWarrior, {});
    assert(deadHealerAssessment.teamPresent);
    assert(!deadHealerAssessment.hasCapableRescuer);
    assert(deadHealerAssessment.rescuerName == "Boros");
    assert(deadHealerAssessment.survivalChancePercent == 15);

    std::vector<RecruitedAllyCombatSupport> healers = {ally("Aline", "Soigneur", 2, 4)};
    const auto healerAssessment = LethalTeamRescueSystem::assessRecruitedTeam(healers, {});
    assert(healerAssessment.teamPresent);
    assert(healerAssessment.hasCapableRescuer);
    assert(healerAssessment.survivalChancePercent >= 33);
    assert(healerAssessment.survivalChancePercent <= 66);
    assert(healerAssessment.rescuerName == "Aline");

    std::vector<RecruitedAllyCombatSupport> eliteHealers = {ally("Mira", "Médecin de terrain", 6, 10)};
    const auto eliteAssessment = LethalTeamRescueSystem::assessRecruitedTeam(eliteHealers, {});
    assert(eliteAssessment.hasCapableRescuer);
    assert(eliteAssessment.survivalChancePercent >= healerAssessment.survivalChancePercent);
    assert(eliteAssessment.survivalChancePercent >= 33);
    assert(eliteAssessment.survivalChancePercent <= 66);

    auto legendaryHealer = ally("Seline", "Médecin de terrain", 6, 10);
    legendaryHealer.estimatedLevel = 255;
    const auto legendaryAssessment = LethalTeamRescueSystem::assessRecruitedTeam({legendaryHealer}, {});
    assert(legendaryAssessment.hasCapableRescuer);
    assert(legendaryAssessment.survivalChancePercent == 66);

    Player recruitedSaved;
    recruitedSaved.takeDamage(recruitedSaved.getMaxHp() * 10);
    assert(recruitedSaved.isDead());
    Random successRandom(seedForSuccess(healerAssessment.survivalChancePercent));
    const auto recruitedResult = LethalTeamRescueSystem::tryRecruitedTeamRescue(
        recruitedSaved, healers, {}, successRandom, "test_recruit");
    assert(recruitedResult.survived);
    assert(!recruitedSaved.isDead());

    Player recruitedLost;
    recruitedLost.takeDamage(recruitedLost.getMaxHp() * 10);
    Random failureRandom(seedForFailure(warriorAssessment.survivalChancePercent));
    const auto failed = LethalTeamRescueSystem::tryRecruitedTeamRescue(
        recruitedLost, warriors, {}, failureRandom, "test_warrior");
    assert(!failed.survived);
    assert(recruitedLost.isDead());

    Player downed("Downed", ClassCatalog::createClassByName("Guerrier"));
    Player priest("Priest", ClassCatalog::createClassByName("Prêtre"));
    Player lumomancer("Lumo", ClassCatalog::createClassByName("Lumomancien"));
    Player warrior("Warrior", ClassCatalog::createClassByName("Guerrier"));
    assert(LethalTeamRescueSystem::humanPlayerCanPerformLethalRescue(priest));
    assert(LethalTeamRescueSystem::humanPlayerCanPerformLethalRescue(lumomancer));
    assert(!LethalTeamRescueSystem::humanPlayerCanPerformLethalRescue(warrior));

    downed.takeDamage(downed.getMaxHp() * 10);
    std::vector<Player*> party = {&downed, &priest, &warrior};
    Random partySuccess(seedForSuccess(50));
    const auto partyResult = LethalTeamRescueSystem::tryHumanPartyRescue(downed, party, partySuccess, "test_party");
    assert(partyResult.teamPresent);
    assert(partyResult.hasCapableRescuer);
    assert(partyResult.survivalChancePercent >= 33 && partyResult.survivalChancePercent <= 66);
    if (partyResult.survived) assert(!downed.isDead());

    return 0;
}
