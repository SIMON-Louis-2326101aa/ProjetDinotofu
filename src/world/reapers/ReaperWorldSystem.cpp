#include "world/reapers/ReaperWorldSystem.hpp"

#include "core/Random.hpp"
#include "entity/Player.hpp"
#include "world/rumor/WorldRumorSystem.hpp"

#include <algorithm>

namespace
{
    int exactCount(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key) return std::max(0, record.count);
        }
        return 0;
    }

    int latestReaperObservationDay(const Player& player)
    {
        int latest = -1000000;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == "reapers13_observation") latest = std::max(latest, record.lastDay);
        }
        return latest;
    }

    bool dangerousForReaperObservation(const std::string& biomeName)
    {
        return biomeName.find("Ruines") != std::string::npos
            || biomeName.find("Quartier abandonné") != std::string::npos
            || biomeName.find("Route") != std::string::npos
            || biomeName.find("Forêt") != std::string::npos
            || biomeName.find("Mine") != std::string::npos
            || biomeName.find("Glacier") != std::string::npos
            || biomeName.find("Temple") != std::string::npos;
    }
}

bool ReaperWorldSystem::hasCredibleKnowledge(const Player& player)
{
    return player.getCanonicalJournalCategoryTotal("reapers13_knowledge") > 0
        || observationScore(player) > 0
        || hasReceivedProtocol13Response(player);
}

bool ReaperWorldSystem::hasPendingProtocol13Signal(const Player& player)
{
    return exactCount(player, "protocoles_secrets", "reapers13_protocol_13_signal") > 0
        && !hasReceivedProtocol13Response(player);
}

bool ReaperWorldSystem::hasReceivedProtocol13Response(const Player& player)
{
    return player.getCanonicalJournalCategoryTotal("reapers13_protocol_responses") > 0;
}

int ReaperWorldSystem::observationScore(const Player& player)
{
    return std::clamp(player.getCanonicalJournalCategoryTotal("reapers13_observation"), 0, 100);
}

ReaperWorldEncounterResult ReaperWorldSystem::tryExplorationEncounter(
    Player& player,
    Random& random,
    const std::string& biomeName)
{
    ReaperWorldEncounterResult result;
    const bool pendingSignal = hasPendingProtocol13Signal(player);
    const bool credibleBefore = hasCredibleKnowledge(player);
    const bool dangerousBiome = dangerousForReaperObservation(biomeName);

    // REAPERS-13 is deliberately a major, optional world faction. Natural sightings are
    // extremely rare, only happen in plausible dangerous areas, and are separated by a long
    // cooldown so the faction cannot crowd out ordinary exploration, companions or the main plot.
    if (!pendingSignal)
    {
        if (!dangerousBiome) return result;
        const int latestObservation = latestReaperObservationDay(player);
        if (player.getWorldDaysElapsed() - latestObservation < 12) return result;
    }
    const int chance = pendingSignal ? 30 : 2;
    if (random.between(1, 100) > chance) return result;

    result.occurred = true;
    result.respondedToProtocol13 = pendingSignal;
    result.recruitmentOffered = false;
    result.encounterId = pendingSignal
        ? "reapers13_protocol_response"
        : "reapers13_natural_field_trace";

    if (pendingSignal)
    {
        result.lines.push_back("Une silhouette équipée sobrement t'attend hors de l'axe principal. Elle ne porte aucun insigne public.");
        result.lines.push_back("« 13-02313 a été reçu. Maintenant la vraie question : comment connais-tu ce canal ? »");
        result.lines.push_back("Le contact se présente seulement comme relais d'une Echo Unit. Il observe, vérifie et repart : aucun recrutement automatique, aucun grade et aucune équipe gratuite.");
        result.lines.push_back("Le signal a donc reçu une réponse réelle dans le monde, mais REAPERS-13 reste une organisation qui sélectionne ses candidats selon leur comportement.");
        player.recordCanonicalEvent(
            "reapers13_protocol_responses",
            "field_response_day_" + std::to_string(player.getWorldDaysElapsed()),
            "Un relais Echo Unit a répondu physiquement au Protocol 13 dans " + biomeName + ".",
            1);
        player.recordCanonicalEvent(
            "reapers13_observation",
            "echo_contact",
            "Contact direct très bref avec un relais Echo Unit.",
            3);
        WorldRumorSystem::confirmRumor(
            player,
            "reapers13_field_presence",
            "REAPERS-13",
            "Un relais Echo Unit a répondu physiquement au signal 13-02313.",
            4);
        player.recordHistoricalEvent(
            "reapers13_field_response",
            biomeName,
            "Le signal 13-02313 a reçu une réponse physique d'un relais Echo Unit, sans recrutement.",
            true);
    }
    else
    {
        result.lines.push_back("Tu remarques une scène trop propre pour un groupe de bandits : traces de lutte concentrées, aucune fouille inutile, aucune marque de pillage.");
        result.lines.push_back("Les traces les plus étranges sont autour des sorties : passage secondaire coupé, angle de retraite surveillé, puis une seule voie laissée ouverte vers l'endroit où la cible pouvait être interceptée.");
        result.lines.push_back("Plus loin, une silhouette surveille brièvement la zone avant de disparaître par un itinéraire déjà préparé. Ce n'est pas une preuve de pouvoirs surnaturels : plutôt le travail de chasseurs qui préparent la fuite avant le combat.");
        result.lines.push_back("Tu n'apprends ni nom, ni grade, ni objectif secret. Seulement la preuve qu'une unité disciplinée de chasse opère réellement dans ce monde.");
        player.recordCanonicalEvent(
            "reapers13_knowledge",
            "field_trace:" + biomeName,
            "Trace crédible d'une unité classifiée observée en exploration.",
            1);
        player.recordCanonicalEvent(
            "reapers13_observation",
            "natural_trace",
            "Observation indirecte d'une opération propre et ciblée.",
            1);
        WorldRumorSystem::confirmRumor(
            player,
            "reapers13_field_presence",
            "REAPERS-13",
            "Une opération inconnue, disciplinée, ciblée et sans pillage inutile a été observée directement.",
            3);
        player.recordHistoricalEvent(
            "reapers13_natural_trace",
            biomeName,
            "Une opération inconnue, disciplinée et sans pillage inutile a été observée sur le terrain.",
            true);
    }

    result.firstCredibleKnowledge = !credibleBefore && hasCredibleKnowledge(player);
    return result;
}
