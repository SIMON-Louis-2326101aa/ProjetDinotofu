#include "entity/Player.hpp"
#include "world/crime/CrimeLocalOpportunitySystem.hpp"

#include <cassert>
#include <iostream>

int main()
{
    Player player;
    player.setCurrentCityId("valebrume");

    assert(CrimeLocalOpportunitySystem::queue(
        player,
        "valebrume",
        "route_watch",
        "valebrume_receleur_brocanteur",
        "Une arrestation perturbe les relais locaux."));
    assert(!CrimeLocalOpportunitySystem::queue(
        player,
        "valebrume",
        "route_watch",
        "autre_source",
        "Le même suivi ne doit pas être dupliqué."));

    const auto entries = CrimeLocalOpportunitySystem::pending(player, "valebrume");
    assert(entries.size() == 1);
    assert(entries[0].kind == "route_watch");

    const CrimeLocalOpportunityAcceptResult accepted = CrimeLocalOpportunitySystem::accept(player, entries[0].eventId);
    assert(accepted.accepted);
    assert(!accepted.questId.empty());
    assert(player.getQuestLog().hasQuest(accepted.questId));
    assert(CrimeLocalOpportunitySystem::pendingCount(player, "valebrume") == 0);

    const Quest& quest = player.getQuestLog().getQuests().front();
    assert(quest.guildQuest);
    assert(quest.objectiveType == "service");
    assert(quest.accepted);

    std::cout << "CrimeLocalOpportunitySystemTest PASS\n";
    return 0;
}
