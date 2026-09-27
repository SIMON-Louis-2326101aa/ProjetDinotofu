#include "world/npc/NpcInformationPropagationSystem.hpp"
#include "entity/Player.hpp"
#include <cassert>

int main()
{
    Player player;
    player.setCurrentCityId("valebrume");
    player.rememberNpcFact(
        "Garde A",
        "local_attack",
        "attaque_route_est",
        "Le garde a vu une troupe inhabituelle sur la route est",
        "observation_directe",
        "Garde A",
        94,
        2,
        "troupe_inconnue",
        "poste_de_garde"
    );

    player.setCurrentCityId("rocheveille");
    const int distance = 35; // Valebrume (0,0) -> Rocheveille (32,-14), ~35 km.
    const int delay = NpcInformationPropagationSystem::intercityTravelDelayDays(distance, "reseau_garde");
    assert(delay >= 1);

    const NpcPropagationResult tooEarly = NpcInformationPropagationSystem::propagateOneIntercityFact(player, "Marchand B");
    assert(!tooEarly.transferred);

    player.advanceWorldDays(delay);
    const NpcPropagationResult arrived = NpcInformationPropagationSystem::propagateOneIntercityFact(player, "Marchand B");
    assert(arrived.transferred);
    assert(arrived.sourceNpc == "Garde A");
    assert(arrived.relayChannel == "messager_de_garde");
    assert(arrived.confidence < 94);
    assert(player.npcKnowsFact("Marchand B", "local_attack", "attaque_route_est"));

    const NpcPropagationResult duplicate = NpcInformationPropagationSystem::propagateOneIntercityFact(player, "Marchand B");
    assert(!duplicate.transferred);

    assert(NpcInformationPropagationSystem::intercityCarrierForNetwork("reseau_commercial") == "caravane_marchande");
    assert(NpcInformationPropagationSystem::intercityTravelDelayDays(80, "reseau_temple")
        > NpcInformationPropagationSystem::intercityTravelDelayDays(10, "reseau_garde"));
    return 0;
}
