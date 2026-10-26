#include "world/bounty/BountyAftermathSystem.hpp"

#include "entity/Player.hpp"
#include "world/bounty/BountyIdentityCatalog.hpp"
#include "world/bounty/BountyRegionalConsequenceSystem.hpp"
#include "quest/Quest.hpp"

#include <algorithm>

namespace
{
    constexpr const char* IssuerCategory = "bounty_aftermath_issuer";
    constexpr const char* OrganizationCategory = "bounty_aftermath_organization";
    constexpr const char* CloseRelationCategory = "bounty_aftermath_close_relation";

    std::string makeSubjectId(const std::string& targetId, BountyResolutionKind resolution)
    {
        return targetId + "|" + BountyRegionalConsequenceSystem::resolutionId(resolution);
    }

    bool splitSubjectId(const std::string& subjectId, std::string& targetId, std::string& resolutionId)
    {
        const std::size_t separator = subjectId.find('|');
        if (separator == std::string::npos || separator == 0 || separator + 1 >= subjectId.size()) return false;
        targetId = subjectId.substr(0, separator);
        resolutionId = subjectId.substr(separator + 1);
        return !targetId.empty() && !resolutionId.empty();
    }

    bool isAliveResolution(const std::string& resolutionId)
    {
        return resolutionId != "killed_combat";
    }

    bool issuerPreferred(const BountyIdentityDefinition& definition, const std::string& resolutionId)
    {
        return isAliveResolution(resolutionId) || !definition.issuerPrefersLiveCapture;
    }

    bool isAftermathCategory(const std::string& category)
    {
        return category == IssuerCategory || category == OrganizationCategory || category == CloseRelationCategory;
    }

    BountyAftermathActorType actorTypeFor(const std::string& category)
    {
        if (category == OrganizationCategory) return BountyAftermathActorType::Organization;
        if (category == CloseRelationCategory) return BountyAftermathActorType::CloseRelation;
        return BountyAftermathActorType::Issuer;
    }

    bool alreadyQueued(const Player& player, const std::string& category, const std::string& subjectId)
    {
        for (const PlayerHistoricalEvent& event : player.getHistoricalEvents())
        {
            if (event.category == category && event.subjectId == subjectId) return true;
        }
        return false;
    }

    const PlayerHistoricalEvent* findPendingEvent(const Player& player, const std::string& eventId)
    {
        for (const PlayerHistoricalEvent& event : player.getHistoricalEvents())
        {
            if (event.id == eventId && isAftermathCategory(event.category) && !event.resolved) return &event;
        }
        return nullptr;
    }

    BountyAftermathEntry buildEntry(const PlayerHistoricalEvent& event)
    {
        BountyAftermathEntry entry;
        entry.eventId = event.id;
        entry.actorType = actorTypeFor(event.category);
        entry.eventDay = event.day;
        if (!splitSubjectId(event.subjectId, entry.targetId, entry.resolutionId)) return entry;

        const BountyIdentityDefinition* definition = BountyIdentityCatalog::findById(entry.targetId);
        if (definition == nullptr) return entry;

        entry.targetName = definition->name;
        entry.targetAlive = isAliveResolution(entry.resolutionId);
        entry.preferredOutcome = issuerPreferred(*definition, entry.resolutionId);

        if (entry.actorType == BountyAftermathActorType::Issuer)
        {
            entry.actorId = definition->issuerId;
            entry.actorName = definition->issuerName;
            entry.lines.push_back(entry.actorName + " a laissé un suivi officiel concernant " + entry.targetName + ".");
            if (entry.preferredOutcome)
                entry.lines.push_back("L'issue correspond à ce que le commanditaire demandait réellement.");
            else
                entry.lines.push_back("Le contrat est clos, mais la mort de la cible ne correspond pas à la capture vivante demandée.");
        }
        else if (entry.actorType == BountyAftermathActorType::Organization)
        {
            entry.actorId = definition->issuerOrganizationId;
            entry.actorName = definition->issuerOrganizationName;
            entry.lines.push_back(entry.actorName + " a enregistré l'issue de la prime sur " + entry.targetName + ".");
            entry.lines.push_back(entry.preferredOutcome
                ? "L'organisation considère que le dossier a été traité conformément à son besoin principal."
                : "L'organisation accepte la clôture, mais garde une réserve sur la manière dont le dossier s'est terminé.");
        }
        else
        {
            entry.actorId = definition->closeRelationId;
            entry.actorName = definition->closeRelationName;
            entry.relationType = definition->closeRelationType;
            entry.lines.push_back(entry.actorName + " (" + entry.relationType + " de " + entry.targetName + ") a fait parvenir un message à la guilde.");
            entry.lines.push_back(entry.targetAlive
                ? "La capture laisse encore une possibilité de parler, d'écrire ou de comprendre ce qui va arriver à la cible."
                : "La mort de la cible rend cette réaction personnelle et durable, même si elle ne change pas rétroactivement le contrat.");
        }
        return entry;
    }

    std::string responseLabel(BountyAftermathResponse response)
    {
        switch (response)
        {
            case BountyAftermathResponse::Professional: return "Répondre de manière professionnelle";
            case BountyAftermathResponse::Conciliatory: return "Reconnaître ce que cette issue a coûté";
            case BountyAftermathResponse::Firm: return "Assumer fermement la décision prise";
            case BountyAftermathResponse::OfferHelp: return "Proposer une aide ou un suivi";
            case BountyAftermathResponse::EndConversation: return "Clore l'échange sans en rajouter";
        }
        return "Répondre";
    }
}

int BountyAftermathSystem::queueResolution(Player& player, const Quest& quest, BountyResolutionKind resolution)
{
    if (!quest.bountyMission || quest.bountyTargetId.empty()) return 0;
    const BountyIdentityDefinition* definition = BountyIdentityCatalog::findById(quest.bountyTargetId);
    if (definition == nullptr) return 0;

    const std::string subjectId = makeSubjectId(definition->id, resolution);
    int queued = 0;

    if (!definition->issuerId.empty() && !alreadyQueued(player, IssuerCategory, subjectId))
    {
        player.recordHistoricalEvent(
            IssuerCategory,
            subjectId,
            definition->issuerName + " attend un retour après la prime sur " + definition->name + ".",
            false);
        ++queued;
    }

    if (!definition->issuerOrganizationId.empty() && !alreadyQueued(player, OrganizationCategory, subjectId))
    {
        player.recordHistoricalEvent(
            OrganizationCategory,
            subjectId,
            definition->issuerOrganizationName + " conserve un suivi du dossier " + definition->name + ".",
            false);
        ++queued;
    }

    if (!definition->closeRelationId.empty() && !alreadyQueued(player, CloseRelationCategory, subjectId))
    {
        player.recordHistoricalEvent(
            CloseRelationCategory,
            subjectId,
            definition->closeRelationName + " a appris l'issue de la prime sur " + definition->name + ".",
            false);
        ++queued;
    }

    if (queued > 0)
    {
        player.recordCanonicalEvent(
            "bounty_aftermath_pending",
            definition->id,
            "Suivis post-prime disponibles pour " + definition->name,
            queued);
    }
    return queued;
}

std::vector<BountyAftermathEntry> BountyAftermathSystem::pending(const Player& player)
{
    std::vector<BountyAftermathEntry> result;
    const int day = player.getWorldDaysElapsed();
    for (const PlayerHistoricalEvent& event : player.getHistoricalEvents())
    {
        if (event.resolved || !isAftermathCategory(event.category)) continue;

        // A close relation is not magically present at the exact instant of the arrest/death.
        // Its message becomes available from the following day; official actors can answer sooner.
        if (event.category == CloseRelationCategory && day <= event.day) continue;

        BountyAftermathEntry entry = buildEntry(event);
        if (entry.actorId.empty() || entry.targetId.empty()) continue;
        result.push_back(std::move(entry));
    }

    std::stable_sort(result.begin(), result.end(), [](const BountyAftermathEntry& a, const BountyAftermathEntry& b) {
        if (a.eventDay != b.eventDay) return a.eventDay < b.eventDay;
        if (a.targetName != b.targetName) return a.targetName < b.targetName;
        return static_cast<int>(a.actorType) < static_cast<int>(b.actorType);
    });
    return result;
}

int BountyAftermathSystem::pendingCount(const Player& player)
{
    return static_cast<int>(pending(player).size());
}

std::vector<BountyAftermathResponseOption> BountyAftermathSystem::responseOptions(const BountyAftermathEntry& entry)
{
    std::vector<BountyAftermathResponseOption> options;
    options.push_back({BountyAftermathResponse::Professional, responseLabel(BountyAftermathResponse::Professional),
        "Rester factuel et traiter le suivi comme une conséquence normale du contrat."});

    if (!entry.preferredOutcome || entry.actorType == BountyAftermathActorType::CloseRelation)
    {
        options.push_back({BountyAftermathResponse::Conciliatory, responseLabel(BountyAftermathResponse::Conciliatory),
            "Ne nie pas les faits et reconnaît la portée humaine ou politique de l'issue."});
    }

    options.push_back({BountyAftermathResponse::Firm, responseLabel(BountyAftermathResponse::Firm),
        "Assumer la décision sans promettre que tout le monde l'approuvera."});

    options.push_back({BountyAftermathResponse::OfferHelp, responseLabel(BountyAftermathResponse::OfferHelp),
        entry.actorType == BountyAftermathActorType::CloseRelation
            ? "Proposer de transmettre une information, un message ou un suivi raisonnable sans réécrire l'issue."
            : "Laisser la porte ouverte à un service ou un contrat futur, sans créer automatiquement une nouvelle quête."});

    options.push_back({BountyAftermathResponse::EndConversation, responseLabel(BountyAftermathResponse::EndConversation),
        "Mettre fin à l'échange proprement, sans bonus ni escalade artificielle."});
    return options;
}

BountyAftermathResolutionResult BountyAftermathSystem::respond(
    Player& player,
    const std::string& eventId,
    BountyAftermathResponse response)
{
    BountyAftermathResolutionResult result;
    const PlayerHistoricalEvent* event = findPendingEvent(player, eventId);
    if (event == nullptr)
    {
        result.lines.push_back("Ce suivi de prime n'est plus disponible.");
        return result;
    }

    BountyAftermathEntry entry = buildEntry(*event);
    if (entry.actorId.empty())
    {
        result.lines.push_back("Le suivi existe dans l'historique, mais son acteur n'est plus identifiable proprement.");
        return result;
    }

    const std::string key = entry.targetId + ":" + entry.actorId;
    switch (response)
    {
        case BountyAftermathResponse::Professional:
            player.recordCanonicalEvent("bounty_aftermath_professional_responses", key,
                "Réponse professionnelle à " + entry.actorName + " après la prime sur " + entry.targetName, 1);
            if (entry.preferredOutcome && entry.actorType != BountyAftermathActorType::CloseRelation)
                player.recordCanonicalEvent("reputation_locale_positive", "suivi_prime_professionnel",
                    "Suivi de prime traité proprement avec " + entry.actorName, 1);
            result.lines.push_back("Tu réponds sans dramatiser ni minimiser : faits, issue et limites du contrat restent clairs.");
            break;

        case BountyAftermathResponse::Conciliatory:
            player.recordCanonicalEvent("bounty_aftermath_conciliatory_responses", key,
                "Réponse conciliante à " + entry.actorName + " après la prime sur " + entry.targetName, 1);
            if (entry.actorType == BountyAftermathActorType::CloseRelation)
                player.recordCanonicalEvent("bounty_close_relation_compassion", entry.targetId,
                    "Le joueur a reconnu la portée personnelle de l'issue auprès de " + entry.actorName, 1);
            result.lines.push_back("Tu reconnais ce que cette issue représente pour l'autre personne sans prétendre pouvoir l'annuler.");
            break;

        case BountyAftermathResponse::Firm:
            player.recordCanonicalEvent("bounty_aftermath_firm_responses", key,
                "Décision assumée face à " + entry.actorName + " après la prime sur " + entry.targetName, 1);
            result.lines.push_back("Tu assumes la décision prise sur le terrain. L'échange reste net, mais il ne garantit aucune sympathie.");
            break;

        case BountyAftermathResponse::OfferHelp:
            if (entry.actorType == BountyAftermathActorType::CloseRelation)
            {
                player.recordCanonicalEvent("bounty_close_relation_requests", entry.targetId,
                    "Aide de suivi proposée à " + entry.actorName + " après la prime sur " + entry.targetName, 1);
                result.lines.push_back(entry.targetAlive
                    ? "Tu proposes de transmettre un message ou une information raisonnable sur la suite de la détention."
                    : "Tu proposes de transmettre ce que tu sais du dernier affrontement sans inventer de consolation automatique.");
            }
            else
            {
                player.recordCanonicalEvent("bounty_aftermath_followup_offers", entry.actorId,
                    "Disponibilité laissée à " + entry.actorName + " pour un futur suivi après " + entry.targetName, 1);
                result.lines.push_back("Tu laisses la porte ouverte à un futur service. Cela crée une opportunité mémorisée, pas une quête gratuite instantanée.");
            }
            break;

        case BountyAftermathResponse::EndConversation:
            player.recordCanonicalEvent("bounty_aftermath_closed_responses", key,
                "Suivi clos sans escalade avec " + entry.actorName, 1);
            result.lines.push_back("Tu clos l'échange sans chercher de récompense, d'excuse artificielle ou de nouvelle confrontation.");
            break;
    }

    player.resolveHistoricalEvent(eventId);
    result.resolved = true;
    result.lines.push_back("Ce suivi est maintenant classé dans l'historique de la prime.");
    return result;
}

std::string BountyAftermathSystem::actorTypeLabel(BountyAftermathActorType actorType)
{
    switch (actorType)
    {
        case BountyAftermathActorType::Issuer: return "commanditaire";
        case BountyAftermathActorType::Organization: return "organisation";
        case BountyAftermathActorType::CloseRelation: return "proche de la cible";
    }
    return "acteur du dossier";
}
