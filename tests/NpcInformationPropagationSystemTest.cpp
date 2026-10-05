#include "world/npc/NpcInformationPropagationSystem.hpp"
#include "world/npc/NpcKnowledgeSystem.hpp"
#include "entity/Player.hpp"
#include <cassert>

int main()
{
    Player player;
    player.setCurrentCityId("ville_test");
    player.rememberNpcFact(
        "Garde A",
        "local_attack",
        "attaque_porte_nord",
        "Le garde affirme avoir vu des gobelins à la porte nord",
        "observation_directe",
        "Garde A",
        92,
        2,
        "gobelins",
        "poste_de_garde"
    );

    const NpcPropagationResult first = NpcInformationPropagationSystem::propagateOneLocalFact(player, "Marchand B");
    assert(first.transferred);
    assert(first.sourceNpc == "Garde A");
    assert(first.confidence < 92);
    assert(first.relayChannel == "marche");
    assert(first.informationNetwork == "reseau_commercial");
    assert(first.claimVariant == "gobelins");
    assert(player.npcKnowsFact("Marchand B", "local_attack", "attaque_porte_nord"));

    // Opening the same contact again must not manufacture repeated certainty
    // from the same relay source and the same version of the claim.
    const NpcPropagationResult duplicate = NpcInformationPropagationSystem::propagateOneLocalFact(player, "Marchand B");
    assert(!duplicate.transferred);

    // A contradictory account about the same event must coexist instead of
    // overwriting the first one as if the world knew which version was true.
    player.rememberNpcFact(
        "Aubergiste C",
        "local_attack",
        "attaque_porte_nord",
        "L'aubergiste jure que les silhouettes portaient des couleurs de bandits",
        "temoignage",
        "voyageur_D",
        84,
        1,
        "bandits",
        "auberge"
    );
    const NpcPropagationResult tooFreshContradiction = NpcInformationPropagationSystem::propagateOneLocalFact(player, "Marchand B");
    assert(!tooFreshContradiction.transferred);
    player.advanceWorldDays(1);
    const NpcPropagationResult contradictory = NpcInformationPropagationSystem::propagateOneLocalFact(player, "Marchand B");
    assert(contradictory.transferred);
    assert(contradictory.claimVariant == "bandits");
    assert(NpcKnowledgeSystem::hasContradictoryClaims(player, "Marchand B", "local_attack", "attaque_porte_nord"));

    const std::vector<NpcKnownFact> merchantFacts = player.getNpcKnownFactsFor("Marchand B", 0);
    int variants = 0;
    for (const NpcKnownFact& fact : merchantFacts)
        if (fact.factType == "local_attack" && fact.subjectId == "attaque_porte_nord") ++variants;
    assert(variants == 2);

    // Local knowledge does not teleport to another city merely because the
    // player changed location.
    player.setCurrentCityId("autre_ville");
    const NpcPropagationResult remote = NpcInformationPropagationSystem::propagateOneLocalFact(player, "Aubergiste E");
    assert(!remote.transferred);
    assert(!player.npcKnowsFact("Aubergiste E", "local_attack", "attaque_porte_nord"));

    assert(NpcInformationPropagationSystem::professionWouldRelay("garde", "rival_seen"));
    assert(NpcInformationPropagationSystem::professionWouldRelay("marchand", "quest_failed"));
    assert(NpcInformationPropagationSystem::professionWouldRelay("guilde", "quest_failed"));
    assert(!NpcInformationPropagationSystem::professionWouldRelay("érudit", "debt_paid"));
    assert(NpcInformationPropagationSystem::relayChannelForProfession("garde") == "poste_de_garde");
    assert(NpcInformationPropagationSystem::relayChannelForProfession("aubergiste") == "auberge");
    assert(NpcInformationPropagationSystem::informationNetworkForProfession("garde") == "reseau_garde");
    assert(NpcInformationPropagationSystem::relayDelayDaysForNetwork("reseau_garde") == 0);
    assert(NpcInformationPropagationSystem::relayDelayDaysForNetwork("reseau_temple") >= 1);

    // A strong local proof weakens a contradictory weak rumor for this NPC only.
    player.setCurrentCityId("ville_test");
    player.rememberNpcFact("Marchand B", "local_attack", "attaque_porte_nord", "Registre et marques confirment des gobelins", "preuve", "registre_garde", 98, 3, "gobelins", "poste_de_garde");
    const std::vector<NpcKnownFact> correctedFacts = player.getNpcKnownFactsFor("Marchand B", 0);
    int goblinConfidence = -1;
    int banditConfidence = -1;
    for (const NpcKnownFact& fact : correctedFacts)
    {
        if (fact.factType != "local_attack" || fact.subjectId != "attaque_porte_nord") continue;
        if (fact.claimVariant == "gobelins") goblinConfidence = fact.confidence;
        if (fact.claimVariant == "bandits") banditConfidence = fact.confidence;
    }
    assert(goblinConfidence >= 98);
    assert(banditConfidence >= 0 && banditConfidence < contradictory.confidence);
    return 0;
}
