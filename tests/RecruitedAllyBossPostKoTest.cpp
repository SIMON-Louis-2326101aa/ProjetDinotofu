#include "combat/ally/RecruitedAllyBossSupport.hpp"
#include "core/Random.hpp"
#include "entity/Boss.hpp"
#include "entity/Player.hpp"

#include <algorithm>
#include <cassert>
#include <vector>

namespace
{
    RecruitedAllyCombatSupport ally(const char* name, const char* job, int level, int rank)
    {
        RecruitedAllyCombatSupport value;
        value.name = name;
        value.job = job;
        value.trait = "calme";
        value.motivation = "tenir le groupe";
        value.estimatedLevel = level;
        value.recruitedLevel = level;
        value.currentRankIndex = rank;
        value.originRankIndex = rank;
        value.maxHp = 140 + level * 2;
        value.currentHp = value.maxHp;
        value.weaponQuality = 5;
        value.armorQuality = 5;
        value.supportKitQuality = 4;
        value.healingPotionCharges = 2;
        value.requestedShare = 15;
        return value;
    }
}

int main()
{
    Player player;
    std::vector<RecruitedAllyCombatSupport> active = {
        ally("Aline", "Soigneur", 35, 4),
        ally("Boros", "Gardien", 38, 4)
    };
    std::vector<RecruitedAllyCombatSupport> reserve = {
        ally("Celia", "Archer", 34, 4),
        ally("Doran", "Assassin", 36, 4)
    };

    Boss normalBoss(90, "Boss test", "test", 70, 8, 14, 18, 0, 0, 2, 2);
    Random random(7u);
    const auto outcome = simulateRecruitedAlliesAfterPlayerDownAgainstBoss(
        player, normalBoss, active, reserve, random, true);
    assert(active.size() <= static_cast<std::size_t>(RecruitedAllyCombatRoster::MaxActiveRecruitsWithoutPlayer));
    assert(outcome.reserveEntries >= 1);
    assert(outcome.playerRecovered || active.empty());
    assert(outcome.simulatedRounds >= 1);
    assert(!outcome.participants.empty());
    for (const auto& participant : outcome.participants)
    {
        assert(participant.turnsTaken > 0 || participant.postKoObservedFromReserve);
        assert(participant.postKoFirstObservedRound >= 1);
        assert(participant.postKoLastObservedRound >= participant.postKoFirstObservedRound);
    }
    assert(normalBoss.getHp() >= 0);

    // Avec trois combattants déjà en ligne, un quatrième peut rester sur le banc pendant tout un
    // boss très court et néanmoins voir directement la scène sans recevoir de participation active.
    std::vector<RecruitedAllyCombatSupport> spectatorActive = {
        ally("FrontA", "Assassin", 80, 6),
        ally("FrontB", "Archer", 80, 6),
        ally("FrontC", "Mage", 80, 6)
    };
    std::vector<RecruitedAllyCombatSupport> spectatorReserve = {
        ally("Gradin", "Observateur", 20, 2)
    };
    Boss tinyBoss(91, "Boss éclair", "test", 1, 1, 1, 1, 0, 0, 1, 1);
    Random spectatorRandom(3u);
    const auto spectatorOutcome = simulateRecruitedAlliesAfterPlayerDownAgainstBoss(
        player, tinyBoss, spectatorActive, spectatorReserve, spectatorRandom, true);
    auto spectatorIt = std::find_if(spectatorOutcome.participants.begin(), spectatorOutcome.participants.end(), [](const auto& participant) {
        return participant.name == "Gradin";
    });
    assert(spectatorIt != spectatorOutcome.participants.end());
    assert(spectatorIt->turnsTaken == 0);
    assert(spectatorIt->postKoObservedFromReserve);
    assert(spectatorIt->postKoObservedRoundsFromReserve >= 1);
    assert(spectatorIt->postKoFirstObservedRound == 1);
    assert(spectatorIt->postKoObservedFinalOutcome);

    // Un défi personnel ne peut jamais être validé par le dernier coup d'une recrue.
    std::vector<RecruitedAllyCombatSupport> fireTeam = {
        ally("Aline", "Archer", 70, 6),
        ally("Boros", "Assassin", 70, 6),
        ally("Celia", "Mage d'appui", 70, 6)
    };
    std::vector<RecruitedAllyCombatSupport> noReserve;
    Boss personalBoss(27, "FireFlight test", "final", 12, 1, 2, 3, 0, 0, 1, 1);
    Random fireRandom(11u);
    const auto fireOutcome = simulateRecruitedAlliesAfterPlayerDownAgainstBoss(
        player, personalBoss, fireTeam, noReserve, fireRandom, false);
    assert(!fireOutcome.bossDefeated);
    assert(!fireOutcome.participants.empty());
    assert(!personalBoss.isDead());
    assert(personalBoss.getHp() >= 1);

    return 0;
}
