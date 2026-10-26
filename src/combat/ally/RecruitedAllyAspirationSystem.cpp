#include "combat/ally/RecruitedAllyAspirationSystem.hpp"

#include "combat/ally/RecruitedAllyCombatRoster.hpp"
#include "entity/Player.hpp"

#include <algorithm>
#include <cctype>

namespace
{
    std::string lower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    int journalCount(const Player& player, const std::string& category, const std::string& key)
    {
        int total = 0;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key) total += std::max(0, record.count);
        }
        return total;
    }
}

RecruitedAllyAspirationSnapshot RecruitedAllyAspirationSystem::evaluate(
    const Player& player,
    const RecruitedAllyCombatSupport& ally)
{
    RecruitedAllyAspirationSnapshot result;
    const std::string text = lower(ally.motivation + " " + ally.job + " " + ally.trait);

    if (text.find("soin") != std::string::npos || text.find("soign") != std::string::npos || text.find("protect") != std::string::npos || text.find("protég") != std::string::npos || text.find("proteg") != std::string::npos || text.find("gard") != std::string::npos)
    {
        result.aspirationId = "protect_group";
        result.aspirationLabel = "protéger le groupe sans sacrifier quelqu'un inutilement";
    }
    else if (text.find("appr") != std::string::npos || text.find("progr") != std::string::npos || text.find("curieux") != std::string::npos)
    {
        result.aspirationId = "grow_stronger";
        result.aspirationLabel = "progresser et devenir plus fiable sur le terrain";
    }
    else if (text.find("gloire") != std::string::npos || text.find("défi") != std::string::npos || text.find("combat") != std::string::npos)
    {
        result.aspirationId = "prove_self";
        result.aspirationLabel = "faire ses preuves dans de vrais combats";
    }
    else if (text.find("argent") != std::string::npos || text.find("prime") != std::string::npos || text.find("part") != std::string::npos)
    {
        result.aspirationId = "fair_reward";
        result.aspirationLabel = "être traité comme un partenaire dont la contribution compte";
    }

    int satisfaction = 45 + RecruitedAllyCombatRoster::relationshipScore(player, ally.name) / 3;
    const int participation = journalCount(player, "participation_recrues", ally.name);
    const int care = journalCount(player, "soins_recuperation_recrues", ally.name)
        + journalCount(player, "soins_relation_recrues", ally.name);
    const int rescues = journalCount(player, "sauvetages_relation_recrues", ally.name);
    const int personalQuest = journalCount(player, "quetes_personnelles_recrues_terminees", ally.name);
    const int refusals = journalCount(player, "recrues_refus_ordres_dangereux", ally.name);

    if (result.aspirationId == "protect_group")
    {
        satisfaction += std::min(18, rescues * 6 + care * 3);
        satisfaction -= std::min(12, refusals * 2);
    }
    else if (result.aspirationId == "grow_stronger")
    {
        satisfaction += std::min(20, participation * 2 + personalQuest * 10);
        satisfaction += std::min(8, care * 2);
    }
    else if (result.aspirationId == "prove_self")
    {
        satisfaction += std::min(22, participation * 3 + personalQuest * 8);
        satisfaction -= std::min(10, refusals * 2);
    }
    else if (result.aspirationId == "fair_reward")
    {
        satisfaction += std::min(14, participation * 2);
        satisfaction += std::min(12, care * 3 + rescues * 3);
    }
    else
    {
        satisfaction += std::min(18, care * 3 + rescues * 5 + personalQuest * 6);
        satisfaction -= std::min(10, refusals * 2);
    }

    result.satisfactionPercent = std::clamp(satisfaction, 0, 100);
    result.moraleModifier = std::clamp((result.satisfactionPercent - 50) / 7, -7, 7);

    const std::string state = result.satisfactionPercent >= 75 ? "aspiration bien nourrie"
        : result.satisfactionPercent >= 50 ? "aspiration stable"
        : result.satisfactionPercent >= 30 ? "aspiration frustrée"
        : "aspiration fortement contrariée";
    result.summary = result.aspirationLabel + " | " + state;
    return result;
}
