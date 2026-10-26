#include "world/bounty/BountySocialConsequenceSystem.hpp"

#include "entity/Player.hpp"
#include "world/bounty/BountyRegionalConsequenceSystem.hpp"
#include "world/bounty/BountyAftermathSystem.hpp"
#include "world/bounty/BountyCustodySystem.hpp"
#include "quest/Quest.hpp"
#include "world/crime/VaultTheftNetworkCatalog.hpp"
#include "world/crime/CrimeLocalImpactSystem.hpp"
#include "world/rumor/WorldRumorSystem.hpp"

#include <algorithm>

namespace
{
    std::string targetLabel(const Quest& quest)
    {
        return quest.bountyTargetName.empty() ? quest.title : quest.bountyTargetName;
    }

    bool capturedAlive(BountyResolutionKind resolution)
    {
        return resolution != BountyResolutionKind::KilledInCombat;
    }
}

BountySocialConsequenceResult BountySocialConsequenceSystem::applyResolution(
    Player& player,
    const Quest& quest,
    BountyResolutionKind resolution)
{
    BountySocialConsequenceResult result;
    if (!quest.bountyMission) return result;
    if (quest.bountyTargetId.empty()
        && quest.bountyIssuerId.empty()
        && quest.bountyCloseRelationId.empty()
        && quest.bountyTargetNetworkId.empty())
    {
        return result;
    }

    result.applied = true;
    const std::string target = targetLabel(quest);
    const bool alive = capturedAlive(resolution);

    if (!quest.bountyIssuerId.empty())
    {
        const bool preferredOutcome = alive || !quest.bountyIssuerPrefersLiveCapture;
        result.issuerSatisfied = preferredOutcome;
        player.recordCanonicalEvent(
            preferredOutcome ? "bounty_issuer_positive_reactions" : "bounty_issuer_mixed_reactions",
            quest.bountyIssuerId,
            (quest.bountyIssuerName.empty() ? std::string("Commanditaire") : quest.bountyIssuerName)
                + " réagit à l'issue de la prime sur " + target
                + " | resolution=" + BountyRegionalConsequenceSystem::resolutionId(resolution),
            1);
        if (!quest.bountyIssuerOrganizationId.empty())
        {
            player.recordCanonicalEvent(
                preferredOutcome ? "bounty_organization_positive_reactions" : "bounty_organization_mixed_reactions",
                quest.bountyIssuerOrganizationId,
                (quest.bountyIssuerOrganizationName.empty() ? std::string("Organisation commanditaire") : quest.bountyIssuerOrganizationName)
                    + " enregistre l'issue de la prime sur " + target,
                1);
        }
        result.lines.push_back(preferredOutcome
            ? "Le commanditaire enregistre une issue conforme à ce qu'il demandait réellement."
            : "Le contrat est clos, mais le commanditaire voulait clairement une capture vivante et n'oublie pas l'issue létale.");
    }

    if (!quest.bountyCloseRelationId.empty())
    {
        result.closeRelationReacted = true;
        const std::string relationName = quest.bountyCloseRelationName.empty()
            ? std::string("Un proche de la cible")
            : quest.bountyCloseRelationName;
        const std::string relationType = quest.bountyCloseRelationType.empty()
            ? std::string("proche")
            : quest.bountyCloseRelationType;
        player.recordCanonicalEvent(
            alive ? "bounty_close_relation_capture_reactions" : "bounty_close_relation_death_reactions",
            quest.bountyCloseRelationId,
            relationName + " (" + relationType + ") apprend l'issue de la prime sur " + target,
            1);
        result.lines.push_back(alive
            ? relationName + " apprend que " + target + " a été capturé vivant : cette relation pourra encore avoir une suite."
            : relationName + " apprend la mort de " + target + " : cette conséquence reste désormais dans l'historique du monde.");
    }

    if (!quest.bountyTargetNetworkId.empty())
    {
        for (PlayerCrimeContactState& state : player.getCrimeContactStates())
        {
            if (state.arrested) continue;
            if (state.networkId.empty())
            {
                const VaultTheftNetworkContact* definition = VaultTheftNetworkCatalog::findById(state.contactId);
                if (definition != nullptr) state.networkId = definition->networkId;
            }
            if (state.networkId != quest.bountyTargetNetworkId) continue;

            state.pressure = std::clamp(state.pressure + (alive ? 7 : 12), 0, 100);
            state.cooperation = std::clamp(state.cooperation - (alive ? 3 : 7), -100, 100);
            state.reliability = std::clamp(state.reliability - (alive ? 1 : 3), 0, 100);
            state.rivalry = std::clamp(state.rivalry + (alive ? 2 : 6), 0, 100);
            ++result.knownNetworkContactsAffected;
        }
        if (result.knownNetworkContactsAffected > 0)
        {
            player.recordCanonicalEvent(
                "bounty_named_network_reactions",
                quest.bountyTargetNetworkId,
                "Le réseau connu lié à " + target + " réagit à la résolution réelle de la prime.",
                result.knownNetworkContactsAffected);
            result.lines.push_back(std::to_string(result.knownNetworkContactsAffected)
                + " contact(s) déjà connu(s) du réseau de la cible modifient réellement leur attitude.");

            const std::string impactCityId = !quest.bountyCurrentCityId.empty()
                ? quest.bountyCurrentCityId
                : player.getCurrentCityId();
            const CrimeLocalImpactRegistration localImpact = CrimeLocalImpactSystem::registerDisruption(
                player,
                impactCityId,
                quest.bountyTargetId,
                alive ? 3 : 4,
                alive
                    ? "La capture d'une cible nommée pousse son réseau connu à modifier ses relais locaux"
                    : "La mort d'une cible nommée provoque une recomposition plus brutale de son réseau connu");
            result.lines.insert(result.lines.end(), localImpact.lines.begin(), localImpact.lines.end());
        }
    }

    if (!quest.bountyTargetId.empty())
    {
        WorldRumorSystem::recordRumor(
            player,
            "named_bounty:" + quest.bountyTargetId,
            "person:" + quest.bountyTargetId,
            alive
                ? target + " a été capturé vivant au terme d'une chasse ciblée."
                : target + " est mort pendant la résolution de sa prime.",
            WorldRumorSourceType::DirectObservation,
            82,
            4);

        const int followUps = BountyAftermathSystem::queueResolution(player, quest, resolution);
        if (followUps > 0)
        {
            result.lines.push_back(std::to_string(followUps)
                + " suivi(s) post-prime sont désormais mémorisés. Les acteurs concernés pourront répondre par l'intermédiaire de la guilde.");
        }
        if (BountyCustodySystem::queueCapture(player, quest, resolution))
        {
            result.lines.push_back("La cible capturée restera disponible pour un unique entretien facultatif à la guilde.");
        }
    }

    return result;
}
