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
    return result;
}

int LocalReputationSystem::score(const Player& player, const std::string& cityId)
{
    return evaluate(player, cityId).score;
}
