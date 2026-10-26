#include "world/bounty/BountyCustodySystem.hpp"

#include "entity/Player.hpp"
#include "quest/Quest.hpp"
#include "world/bounty/BountyIdentityCatalog.hpp"
#include "world/bounty/BountyRegionalConsequenceSystem.hpp"
#include "world/crime/VaultTheftNetworkCatalog.hpp"
#include "world/rumor/WorldRumorSystem.hpp"

#include <algorithm>
#include <cctype>

namespace
{
    constexpr const char* Category = "bounty_custody_interview";

    std::string subjectId(const std::string& targetId, BountyResolutionKind resolution)
    {
        return targetId + "|" + BountyRegionalConsequenceSystem::resolutionId(resolution);
    }

    bool splitSubject(const std::string& value, std::string& targetId, std::string& resolutionId)
    {
        const std::size_t separator = value.find('|');
        if (separator == std::string::npos || separator == 0 || separator + 1 >= value.size()) return false;
        targetId = value.substr(0, separator);
        resolutionId = value.substr(separator + 1);
        return !targetId.empty() && !resolutionId.empty();
    }

    bool isLiveCapture(BountyResolutionKind resolution)
    {
        return resolution != BountyResolutionKind::KilledInCombat;
    }

    const PlayerHistoricalEvent* findPending(const Player& player, const std::string& eventId)
    {
        for (const PlayerHistoricalEvent& event : player.getHistoricalEvents())
        {
            if (event.id == eventId && event.category == Category && !event.resolved) return &event;
        }
        return nullptr;
    }

    bool alreadyQueued(const Player& player, const std::string& subject)
    {
        for (const PlayerHistoricalEvent& event : player.getHistoricalEvents())
        {
            if (event.category == Category && event.subjectId == subject) return true;
        }
        return false;
    }

    std::vector<PlayerCrimeContactState*> knownContactsInNetwork(Player& player, const std::string& networkId)
    {
        std::vector<PlayerCrimeContactState*> contacts;
        if (networkId.empty()) return contacts;
        for (PlayerCrimeContactState& state : player.getCrimeContactStates())
        {
            std::string knownNetwork = state.networkId;
            if (knownNetwork.empty())
            {
                const VaultTheftNetworkContact* definition = VaultTheftNetworkCatalog::findById(state.contactId);
                if (definition != nullptr) knownNetwork = definition->networkId;
            }
            if (knownNetwork == networkId) contacts.push_back(&state);
        }
        return contacts;
    }

    bool likelyToCooperate(const BountyIdentityDefinition& definition, const std::string& resolutionId)
    {
        if (resolutionId == "captured_surrender") return true;
        std::string temperament = definition.temperament;
        std::transform(temperament.begin(), temperament.end(), temperament.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return temperament.find("prudent") != std::string::npos
            || temperament.find("surviv") != std::string::npos
            || temperament.find("méfi") != std::string::npos
            || temperament.find("method") != std::string::npos
            || temperament.find("méthod") != std::string::npos;
    }
}

bool BountyCustodySystem::queueCapture(Player& player, const Quest& quest, BountyResolutionKind resolution)
{
    if (!quest.bountyMission || quest.bountyTargetId.empty() || !isLiveCapture(resolution)) return false;
    const BountyIdentityDefinition* definition = BountyIdentityCatalog::findById(quest.bountyTargetId);
    if (definition == nullptr) return false;

    const std::string subject = subjectId(definition->id, resolution);
    if (alreadyQueued(player, subject)) return false;

    player.recordHistoricalEvent(
        Category,
        subject,
        definition->name + " est disponible pour un entretien unique après sa capture vivante.",
        false);
    player.recordCanonicalEvent(
        "bounty_custody_interviews_available",
        definition->id,
        "Entretien de garde disponible après la capture de " + definition->name,
        1);
    return true;
}

std::vector<BountyCustodyEntry> BountyCustodySystem::pending(const Player& player)
{
    std::vector<BountyCustodyEntry> result;
    for (const PlayerHistoricalEvent& event : player.getHistoricalEvents())
    {
        if (event.category != Category || event.resolved) continue;

        BountyCustodyEntry entry;
        if (!splitSubject(event.subjectId, entry.targetId, entry.resolutionId)) continue;
        const BountyIdentityDefinition* definition = BountyIdentityCatalog::findById(entry.targetId);
        if (definition == nullptr) continue;

        entry.eventId = event.id;
        entry.targetName = definition->name;
        entry.temperament = definition->temperament;
        entry.networkId = definition->networkId;
        entry.captureDay = event.day;
        entry.lines.push_back(entry.targetName + " est détenu(e) après une capture vivante.");
        entry.lines.push_back("L'entretien est facultatif et unique : refuser ou partir ne bloque pas la prime déjà terminée.");
        entry.lines.push_back("Toute déclaration reste un témoignage à recouper, jamais une vérité automatique.");
        result.push_back(entry);
    }
    return result;
}

int BountyCustodySystem::pendingCount(const Player& player)
{
    return static_cast<int>(pending(player).size());
}

std::vector<BountyCustodyOption> BountyCustodySystem::options(const BountyCustodyEntry&)
{
    return {
        {BountyCustodyApproach::CalmDebrief,
            "Revenir calmement sur les faits",
            "Chercher un récit exploitable sans promettre qu'il soit vrai."},
        {BountyCustodyApproach::AskAboutNetwork,
            "Questionner sur ses relais et son réseau",
            "Peut donner une piste sur des contacts déjà connus, jamais révéler gratuitement un inconnu."},
        {BountyCustodyApproach::OfferFormalCooperation,
            "Proposer une coopération formelle",
            "La cible peut accepter ou refuser. Une reddition volontaire facilite fortement cet accord."}
    };
}

BountyCustodyResult BountyCustodySystem::resolve(
    Player& player,
    const std::string& eventId,
    BountyCustodyApproach approach)
{
    BountyCustodyResult result;
    const PlayerHistoricalEvent* event = findPending(player, eventId);
    if (event == nullptr)
    {
        result.lines.push_back("Cet entretien n'est plus disponible ou a déjà été traité.");
        return result;
    }

    std::string targetId;
    std::string resolutionId;
    if (!splitSubject(event->subjectId, targetId, resolutionId))
    {
        result.lines.push_back("Le dossier de garde est incomplet.");
        return result;
    }
    const BountyIdentityDefinition* definition = BountyIdentityCatalog::findById(targetId);
    if (definition == nullptr)
    {
        result.lines.push_back("L'identité liée à ce dossier n'est plus disponible dans le catalogue.");
        return result;
    }

    const bool surrendered = resolutionId == "captured_surrender";
    if (approach == BountyCustodyApproach::CalmDebrief)
    {
        const std::string testimony = definition->name
            + " donne sa propre version de la fuite, de ses décisions et des personnes qu'il ou elle prétendait éviter.";
        WorldRumorSystem::recordRumor(
            player,
            "bounty_testimony:" + targetId,
            "person:" + targetId,
            testimony,
            WorldRumorSourceType::Witness,
            surrendered ? 58 : 46,
            1);
        player.recordCanonicalEvent(
            "bounty_custody_personal_interviews",
            targetId,
            "Entretien calme avec " + definition->name,
            1);
        result.testimonyRecorded = true;
        result.lines.push_back("La cible accepte de raconter sa version des faits.");
        result.lines.push_back("Le journal l'enregistre comme témoignage : il faudra le recouper avant de le considérer comme établi.");
    }
    else if (approach == BountyCustodyApproach::AskAboutNetwork)
    {
        std::vector<PlayerCrimeContactState*> contacts = knownContactsInNetwork(player, definition->networkId);
        for (PlayerCrimeContactState* state : contacts)
        {
            if (state == nullptr || state->arrested) continue;
            state->pressure = std::clamp(state->pressure + (surrendered ? 5 : 3), 0, 100);
            state->rivalry = std::clamp(state->rivalry + 1, 0, 100);
            ++result.knownNetworkContactsAffected;
        }
        WorldRumorSystem::recordRumor(
            player,
            "bounty_network_testimony:" + targetId,
            "network:" + definition->networkId,
            definition->name + " affirme connaître certaines habitudes de circulation de son réseau.",
            WorldRumorSourceType::Witness,
            surrendered ? 62 : 48,
            1);
        player.recordCanonicalEvent(
            "bounty_custody_network_leads",
            targetId,
            "Déclaration sur un réseau après capture de " + definition->name,
            std::max(1, result.knownNetworkContactsAffected));
        result.testimonyRecorded = true;
        if (result.knownNetworkContactsAffected > 0)
        {
            result.lines.push_back(std::to_string(result.knownNetworkContactsAffected)
                + " contact(s) déjà connu(s) du même réseau deviennent plus prudents après les recoupements possibles.");
        }
        else
        {
            result.lines.push_back("La cible parle de méthodes et de relais, mais aucun contact correspondant n'est déjà connu du joueur.");
            result.lines.push_back("Aucun nom inconnu n'est révélé automatiquement : cette déclaration reste une piste générale.");
        }
    }
    else
    {
        result.cooperated = likelyToCooperate(*definition, resolutionId);
        if (result.cooperated)
        {
            std::vector<PlayerCrimeContactState*> contacts = knownContactsInNetwork(player, definition->networkId);
            for (PlayerCrimeContactState* state : contacts)
            {
                if (state == nullptr || state->arrested) continue;
                state->pressure = std::clamp(state->pressure + 6, 0, 100);
                state->cooperation = std::clamp(state->cooperation - 2, -100, 100);
                ++result.knownNetworkContactsAffected;
            }
            player.recordCanonicalEvent(
                "bounty_custody_formal_cooperation",
                targetId,
                definition->name + " accepte une coopération formelle limitée",
                1);
            result.lines.push_back(definition->name + " accepte de consigner une coopération limitée auprès de la guilde et de la garde.");
            result.lines.push_back("Cela n'annule ni sa capture ni sa responsabilité : l'accord porte uniquement sur les informations qu'il ou elle accepte de fournir.");
        }
        else
        {
            player.recordCanonicalEvent(
                "bounty_custody_cooperation_refused",
                targetId,
                definition->name + " refuse une coopération formelle",
                1);
            result.lines.push_back(definition->name + " refuse tout accord formel et préfère laisser la procédure suivre son cours.");
            result.lines.push_back("Le refus n'est pas transformé en aveu et ne crée aucune preuve supplémentaire.");
        }
    }

    player.resolveHistoricalEvent(eventId);
    player.recordHistoricalEvent(
        "bounty_custody_interview_resolved",
        targetId,
        "Entretien post-capture traité avec " + definition->name + ".",
        true);
    result.resolved = true;
    return result;
}
