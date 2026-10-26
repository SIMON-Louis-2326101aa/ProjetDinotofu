#include "combat/ally/RecruitedAllyAspirationSystem.hpp"
#include "combat/ally/RecruitedAllyCombatRoster.hpp"
#include "entity/Player.hpp"

#include <cassert>
#include <iostream>

int main()
{
    Player player;
    RecruitedAllyCombatSupport healer;
    healer.name = "Aelia";
    healer.job = "soigneuse de terrain";
    healer.motivation = "protéger les autres";
    healer.teamOrigin = AllyTeamOrigin::PersonalRecruit;

    const auto base = RecruitedAllyAspirationSystem::evaluate(player, healer);
    assert(base.aspirationId == "protect_group");

    player.recordCanonicalEvent("participation_recrues", healer.name, "combat partagé", 4);
    player.recordCanonicalEvent("sauvetages_relation_recrues", healer.name, "allié sauvé", 2);
    player.recordCanonicalEvent("soins_recuperation_recrues", healer.name, "récupération accompagnée", 2);
    const auto supported = RecruitedAllyAspirationSystem::evaluate(player, healer);
    assert(supported.satisfactionPercent > base.satisfactionPercent);
    assert(supported.moraleModifier >= base.moraleModifier);

    RecruitedAllyCombatSupport ambitious;
    ambitious.name = "Rook";
    ambitious.job = "éclaireur";
    ambitious.motivation = "progresser et apprendre";
    ambitious.teamOrigin = AllyTeamOrigin::PersonalRecruit;
    const auto growth = RecruitedAllyAspirationSystem::evaluate(player, ambitious);
    assert(growth.aspirationId == "grow_stronger");

    std::cout << "RecruitedAllyAspirationSystemTest OK\n";
    return 0;
}
