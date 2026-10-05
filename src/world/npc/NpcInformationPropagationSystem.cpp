#include "world/npc/NpcInformationPropagationSystem.hpp"
#include "world/npc/NpcKnowledgeSystem.hpp"
#include "world/npc/NpcRelationshipSystem.hpp"
#include "entity/Player.hpp"
#include "world/CityTravelRules.hpp"
#include "world/City.hpp"

#include <algorithm>

namespace
{
bool recipientAlreadyHasThisSource(const Player& player, const std::string& npcId, const NpcKnownFact& sourceFact)
{
    for (const NpcKnownFact& known : player.getNpcKnownFactsFor(npcId, 0))
    {
        if (known.factType == sourceFact.factType
            && known.subjectId == sourceFact.subjectId
            && known.claimVariant == sourceFact.claimVariant
            && known.sourceId == sourceFact.npcId)
        {
            return true;
        }
    }
    return false;
}

int relayPriority(const std::string& recipientName, const LivingNpcProfile& recipient, const std::string& sourceName, const LivingNpcProfile& source, const NpcKnownFact& fact, int effectiveConfidence)
{
    int score = effectiveConfidence + fact.evidenceLevel * 8;
    if (NpcInformationPropagationSystem::professionWouldRelay(recipient.profession, fact.factType)) score += 24;
    if (!recipient.informationNetwork.empty() && recipient.informationNetwork == source.informationNetwork) score += 14;
    if (fact.locationId.empty()) score -= 8;
    if (fact.sourceType == "rumeur" || fact.sourceType == "rumeur_locale") score -= 12;
    score += NpcRelationshipSystem::relayModifier(recipientName, sourceName);
    return score;
}

int relayConfidencePenaltyForNetwork(const std::string& networkId)
{
    if (networkId == "reseau_garde") return 12;
    if (networkId == "reseau_guilde") return 13;
    if (networkId == "reseau_savant") return 14;
    if (networkId == "reseau_contacts") return 15;
    if (networkId == "reseau_commercial") return 18;
    if (networkId == "reseau_temple") return 18;
    if (networkId == "reseau_auberges") return 22;
    return 25;
}
}

bool NpcInformationPropagationSystem::professionWouldRelay(const std::string& profession, const std::string& factType)
{
    if (profession == "garde")
        return factType == "rival_seen" || factType == "local_attack" || factType == "unusual_monster";
    if (profession == "marchand" || profession == "marchand itinérant")
        return factType == "rival_seen" || factType == "local_attack" || factType == "unusual_monster" || factType == "debt_paid" || factType == "quest_failed";
    if (profession == "intendance")
        return factType == "rival_seen" || factType == "local_attack" || factType == "quest_completed" || factType == "quest_failed" || factType == "unusual_monster";
    if (profession == "soigneuse")
        return factType == "local_attack" || factType == "unusual_monster" || factType == "quest_completed";
    if (profession == "logistique")
        return factType == "local_attack" || factType == "debt_paid" || factType == "quest_completed" || factType == "quest_failed";
    if (profession == "messagère")
        return true;
    if (profession == "érudit")
        return factType == "discovered_writing" || factType == "unusual_monster";
    if (profession == "guilde")
        return factType == "rival_seen" || factType == "local_attack" || factType == "quest_completed" || factType == "quest_failed" || factType == "unusual_monster";
    if (profession == "religieux")
        return factType == "local_attack" || factType == "quest_completed" || factType == "quest_failed";
    if (profession == "aubergiste" || profession == "contact")
        return true;
    return factType == "local_attack" || factType == "rival_seen";
}

std::string NpcInformationPropagationSystem::relayChannelForProfession(const std::string& profession)
{
    if (profession == "garde") return "poste_de_garde";
    if (profession == "marchand" || profession == "marchand itinérant") return "marche";
    if (profession == "intendance") return "bureau_intendance";
    if (profession == "soigneuse") return "reseau_de_soins";
    if (profession == "logistique") return "registre_logistique";
    if (profession == "messagère") return "courrier_local";
    if (profession == "érudit") return "bibliotheque";
    if (profession == "guilde") return "guilde";
    if (profession == "religieux") return "temple";
    if (profession == "aubergiste") return "auberge";
    if (profession == "contact") return "reseau_de_contacts";
    return "bouche_a_oreille";
}

std::string NpcInformationPropagationSystem::informationNetworkForProfession(const std::string& profession)
{
    if (profession == "garde") return "reseau_garde";
    if (profession == "marchand" || profession == "marchand itinérant") return "reseau_commercial";
    if (profession == "intendance" || profession == "messagère") return "reseau_guilde";
    if (profession == "soigneuse") return "reseau_temple";
    if (profession == "logistique") return "reseau_commercial";
    if (profession == "érudit") return "reseau_savant";
    if (profession == "guilde") return "reseau_guilde";
    if (profession == "religieux") return "reseau_temple";
    if (profession == "aubergiste") return "reseau_auberges";
    if (profession == "contact") return "reseau_contacts";
    if (profession == "artisan") return "reseau_artisans";
    return "voisinage";
}

int NpcInformationPropagationSystem::relayDelayDaysForNetwork(const std::string& networkId)
{
    if (networkId == "reseau_garde" || networkId == "reseau_guilde" || networkId == "reseau_contacts") return 0;
    if (networkId == "reseau_commercial" || networkId == "reseau_savant" || networkId == "reseau_auberges") return 1;
    if (networkId == "reseau_artisans" || networkId == "reseau_temple") return 2;
    return 2;
}


std::string NpcInformationPropagationSystem::intercityCarrierForNetwork(const std::string& networkId)
{
    if (networkId == "reseau_garde") return "messager_de_garde";
    if (networkId == "reseau_guilde") return "courrier_de_guilde";
    if (networkId == "reseau_commercial") return "caravane_marchande";
    if (networkId == "reseau_savant") return "copie_archivee";
    if (networkId == "reseau_temple") return "pelerin_itinerant";
    if (networkId == "reseau_auberges") return "voyageur_de_relais";
    if (networkId == "reseau_artisans") return "convoi_artisan";
    if (networkId == "reseau_contacts") return "coursier_de_contacts";
    return "voyageur_ordinaire";
}

int NpcInformationPropagationSystem::intercityTravelDelayDays(int distanceKm, const std::string& networkId)
{
    if (distanceKm < 0) return 999;
    int days = 1;
    if (distanceKm > 20) days = 2;
    if (distanceKm > 45) days = 3;
    if (distanceKm > 70) days = 4;

    if (networkId == "reseau_garde" || networkId == "reseau_guilde" || networkId == "reseau_contacts")
        days = std::max(1, days - 1);
    else if (networkId == "reseau_savant" || networkId == "reseau_temple")
        days += 1;

    return days;
}

NpcPropagationResult NpcInformationPropagationSystem::propagateOneLocalFact(Player& player, const std::string& recipientNpc)
{
    NpcPropagationResult result;
    if (recipientNpc.empty()) return result;

    LivingNpcProfile recipientProfile = NpcKnowledgeSystem::profileForNamedNpc(recipientNpc);
    if (recipientProfile.informationNetwork.empty()) recipientProfile.informationNetwork = informationNetworkForProfession(recipientProfile.profession);
    if (!recipientProfile.remembersLocalEvents) return result;

    const std::string currentLocation = player.getCurrentCityId();
    const int currentDay = player.getWorldDaysElapsed();
    const NpcKnownFact* best = nullptr;
    int bestScore = -100000;
    int bestEffectiveConfidence = 0;

    const std::vector<NpcKnownFact>& allFacts = player.getNpcKnownFacts();
    for (const NpcKnownFact& fact : allFacts)
    {
        if (fact.npcId.empty() || fact.npcId == recipientNpc) continue;
        if (!currentLocation.empty() && !fact.locationId.empty() && fact.locationId != currentLocation) continue;
        if (recipientAlreadyHasThisSource(player, recipientNpc, fact)) continue;

        LivingNpcProfile sourceProfile = NpcKnowledgeSystem::profileForNamedNpc(fact.npcId);
        if (sourceProfile.informationNetwork.empty()) sourceProfile.informationNetwork = informationNetworkForProfession(sourceProfile.profession);

        const int factAge = std::max(0, currentDay - fact.lastReinforcedDay);
        const int relayDelay = relayDelayDaysForNetwork(sourceProfile.informationNetwork);
        if (factAge < relayDelay) continue;

        const int effective = NpcKnowledgeSystem::effectiveConfidence(fact, currentDay);
        if (effective < 55) continue;
        if (!professionWouldRelay(recipientProfile.profession, fact.factType) && effective < 78) continue;

        const int score = relayPriority(recipientNpc, recipientProfile, fact.npcId, sourceProfile, fact, effective) + std::min(8, fact.timesHeard * 2);
        if (score > bestScore)
        {
            best = &fact;
            bestScore = score;
            bestEffectiveConfidence = effective;
        }
    }

    if (best == nullptr) return result;

    // Copy before writing into Player: rememberNpcFact may reallocate the
    // underlying vector and invalidate pointers/references into it.
    const NpcKnownFact selected = *best;
    LivingNpcProfile selectedSourceProfile = NpcKnowledgeSystem::profileForNamedNpc(selected.npcId);
    if (selectedSourceProfile.informationNetwork.empty()) selectedSourceProfile.informationNetwork = informationNetworkForProfession(selectedSourceProfile.profession);
    const int relayPenalty = relayConfidencePenaltyForNetwork(selectedSourceProfile.informationNetwork);
    const int relayedConfidence = std::clamp(bestEffectiveConfidence - relayPenalty, 25, 88);
    const int relayedEvidence = std::min(1, selected.evidenceLevel);
    player.rememberNpcFact(
        recipientNpc,
        selected.factType,
        selected.subjectId,
        selected.label,
        "rumeur_locale",
        selected.npcId,
        relayedConfidence,
        relayedEvidence,
        selected.claimVariant,
        relayChannelForProfession(recipientProfile.profession)
    );

    result.transferred = true;
    result.sourceNpc = selected.npcId;
    result.factType = selected.factType;
    result.subjectId = selected.subjectId;
    result.confidence = relayedConfidence;
    result.relayChannel = relayChannelForProfession(recipientProfile.profession);
    result.informationNetwork = recipientProfile.informationNetwork;
    result.claimVariant = selected.claimVariant;
    result.lines = {
        recipientNpc + " a entendu parler de ce sujet par " + selected.npcId + ".",
        "La transmission reste locale et perd en certitude : ce PNJ connaît désormais une rumeur sourcée, pas la vérité globale.",
        "Canal : " + result.relayChannel + " | réseau : " + result.informationNetwork + ". La version transmise reste « " + result.claimVariant + " » au lieu de fusionner avec une version contradictoire.",
        "Fiabilité transmise : " + NpcKnowledgeSystem::confidenceLabel(relayedConfidence, relayedEvidence) + "."
    };
    const NpcRelationship relation = NpcRelationshipSystem::between(recipientNpc, selected.npcId);
    if (relation.known) result.lines.push_back("Relation entre les deux PNJ : " + relation.type + " — " + relation.description);
    return result;
}


NpcPropagationResult NpcInformationPropagationSystem::propagateOneIntercityFact(Player& player, const std::string& recipientNpc)
{
    NpcPropagationResult result;
    if (recipientNpc.empty()) return result;

    const std::string destinationCity = player.getCurrentCityId();
    if (City::findById(destinationCity) == nullptr) return result;

    LivingNpcProfile recipientProfile = NpcKnowledgeSystem::profileForNamedNpc(recipientNpc);
    if (recipientProfile.informationNetwork.empty()) recipientProfile.informationNetwork = informationNetworkForProfession(recipientProfile.profession);
    if (!recipientProfile.remembersLocalEvents) return result;

    const int currentDay = player.getWorldDaysElapsed();
    const NpcKnownFact* best = nullptr;
    int bestScore = -100000;
    int bestEffectiveConfidence = 0;
    int bestDistance = -1;
    std::string bestSourceNetwork;

    for (const NpcKnownFact& fact : player.getNpcKnownFacts())
    {
        if (fact.npcId.empty() || fact.npcId == recipientNpc) continue;
        if (fact.locationId.empty() || fact.locationId == destinationCity) continue;
        if (City::findById(fact.locationId) == nullptr) continue;
        if (recipientAlreadyHasThisSource(player, recipientNpc, fact)) continue;

        LivingNpcProfile sourceProfile = NpcKnowledgeSystem::profileForNamedNpc(fact.npcId);
        if (sourceProfile.informationNetwork.empty()) sourceProfile.informationNetwork = informationNetworkForProfession(sourceProfile.profession);

        const int distance = CityTravelRules::getDistanceBetweenCities(fact.locationId, destinationCity);
        if (distance < 0) continue;
        const int delay = intercityTravelDelayDays(distance, sourceProfile.informationNetwork);
        const int factAge = std::max(0, currentDay - fact.lastReinforcedDay);
        if (factAge < delay) continue;

        const int effective = NpcKnowledgeSystem::effectiveConfidence(fact, currentDay);
        if (effective < 66) continue;
        if (!professionWouldRelay(recipientProfile.profession, fact.factType) && effective < 84) continue;

        int score = relayPriority(recipientNpc, recipientProfile, fact.npcId, sourceProfile, fact, effective);
        score -= std::max(0, distance / 5);
        if (recipientProfile.informationNetwork == sourceProfile.informationNetwork) score += 10;
        if (fact.evidenceLevel >= 2 && sourceProfile.informationNetwork == "reseau_savant") score += 8;

        if (score > bestScore)
        {
            best = &fact;
            bestScore = score;
            bestEffectiveConfidence = effective;
            bestDistance = distance;
            bestSourceNetwork = sourceProfile.informationNetwork;
        }
    }

    if (best == nullptr) return result;

    const NpcKnownFact selected = *best;
    const std::string carrier = intercityCarrierForNetwork(bestSourceNetwork);
    const int distancePenalty = std::clamp(bestDistance / 4, 4, 22);
    const int networkPenalty = relayConfidencePenaltyForNetwork(bestSourceNetwork);
    const int relayedConfidence = std::clamp(bestEffectiveConfidence - networkPenalty - distancePenalty, 24, 82);
    int relayedEvidence = std::min(1, selected.evidenceLevel);
    std::string sourceType = "rumeur_interville";
    if ((bestSourceNetwork == "reseau_savant" || bestSourceNetwork == "reseau_guilde") && selected.evidenceLevel >= 2)
    {
        relayedEvidence = 2;
        sourceType = "rapport_interville";
    }

    player.rememberNpcFact(
        recipientNpc,
        selected.factType,
        selected.subjectId,
        selected.label,
        sourceType,
        selected.npcId,
        relayedConfidence,
        relayedEvidence,
        selected.claimVariant,
        carrier
    );

    result.transferred = true;
    result.sourceNpc = selected.npcId;
    result.factType = selected.factType;
    result.subjectId = selected.subjectId;
    result.confidence = relayedConfidence;
    result.relayChannel = carrier;
    result.informationNetwork = recipientProfile.informationNetwork;
    result.claimVariant = selected.claimVariant;
    result.lines = {
        "Une information venue de " + selected.locationId + " a atteint " + destinationCity + " par " + carrier + ".",
        "Distance de circulation : " + std::to_string(bestDistance) + " km. Le délai a dû être réellement écoulé avant que le rapport puisse apparaître ici.",
        "Source conservée : " + selected.npcId + ". La version « " + result.claimVariant + " » reste distincte des récits contradictoires.",
        "Fiabilité à l'arrivée : " + NpcKnowledgeSystem::confidenceLabel(relayedConfidence, relayedEvidence) + "."
    };
    const NpcRelationship relation = NpcRelationshipSystem::between(recipientNpc, selected.npcId);
    if (relation.known) result.lines.push_back("Relation entre les deux PNJ : " + relation.type + " — " + relation.description);
    return result;
}
