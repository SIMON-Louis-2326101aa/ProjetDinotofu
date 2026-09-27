#include "world/LocalReputationSystem.hpp"
#include "entity/Player.hpp"

#include <algorithm>

std::string LocalReputationSystem::labelForScore(int score)
{
    if (score >= 80) return "héros local";
    if (score >= 45) return "fiable";
    if (score >= 20) return "apprécié";
    if (score >= 8) return "connu";
    if (score <= -35) return "indésirable";
    if (score < 0) return "suspect";
    return "neutre";
}

int LocalReputationSystem::discountForScore(int score)
{
    if (score >= 80) return 15;
    if (score >= 45) return 12;
    if (score >= 28) return 8;
    if (score >= 14) return 5;
    return 0;
}

int LocalReputationSystem::surchargeForScore(int score)
{
    if (score <= -50) return 14;
    if (score <= -35) return 10;
    if (score <= -18) return 6;
    if (score < 0) return 3;
    return 0;
}

std::string LocalReputationSystem::reactionForScore(int score)
{
    if (score >= 80) return "Les habitants te reconnaissent avant même que tu présentes tes preuves.";
    if (score >= 45) return "Les services locaux prennent ta parole au sérieux.";
    if (score >= 20) return "Quelques visages te saluent et les comptoirs se montrent plus souples.";
    if (score >= 8) return "Ton nom circule un peu, sans ouvrir toutes les portes.";
    if (score <= -50) return "Les volets se ferment plus vite et les services protégés refusent les gros risques.";
    if (score <= -35) return "Les gardes et commerçants vérifient chaque demande avant de répondre.";
    if (score <= -18) return "Les prix se tendent et les marchandises importantes restent derrière le comptoir.";
    if (score < 0) return "Les conversations ralentissent quand tu approches : la ville reste méfiante.";
    return "La ville ne te doit rien et ne te reproche encore rien.";
}

LocalReputationResult LocalReputationSystem::evaluate(const Player& player, const std::string& cityId)
{
    LocalReputationResult result;
    if (cityId.empty()) return result;

    for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
    {
        if (record.locationId != cityId || record.count <= 0) continue;

        if (record.category == "pnj_servis") result.score += record.count * 3;
        else if (record.category == "types_quetes_completees") result.score += record.count * 2;
        else if (record.category == "lieux_visites") result.score += record.count;
        else if (record.category == "coffres_achetes" || record.category == "coffres_ameliores") result.score += record.count;
        else if (record.category == "reputation_locale_positive") result.score += record.count;
        else if (record.category == "reputation_locale_negative") result.score -= record.count;
        else if (record.category == "services_locaux_reussis")
        {
            result.successfulPersonalServices += record.count;
            result.score += record.count * 3;
        }
        else if (record.category == "services_locaux_echoues")
        {
            result.failedPersonalServices += record.count;
            result.score -= record.count * 3;
        }
        else if (record.category == "avertissements_locaux")
        {
            result.warningNotes += record.count;
            result.score -= record.count * 2;
        }
    }

    result.label = labelForScore(result.score);
    result.discountPercent = discountForScore(result.score);
    result.surchargePercent = surchargeForScore(result.score);
    result.reactionLine = reactionForScore(result.score);
    return result;
}

int LocalReputationSystem::score(const Player& player, const std::string& cityId)
{
    return evaluate(player, cityId).score;
}
