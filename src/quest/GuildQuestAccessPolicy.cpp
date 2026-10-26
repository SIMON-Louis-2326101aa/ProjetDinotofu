#include "quest/GuildQuestAccessPolicy.hpp"

#include "combat/ally/RecruitedAllyCombatRoster.hpp"
#include "entity/Player.hpp"
#include "quest/Quest.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace GuildQuestAccessPolicy
{
    int rankIndex(const std::string& rank)
    {
        if (rank.find("Dieu") != std::string::npos) return 11;
        if (rank.find("Légende") != std::string::npos || rank.find("Legende") != std::string::npos) return 10;
        if (rank.find("Héros mondial") != std::string::npos || rank.find("Heros mondial") != std::string::npos) return 9;
        if (rank.find("SSS") != std::string::npos) return 8;
        if (rank.find("SS") != std::string::npos) return 7;
        if (rank.find("S") != std::string::npos) return 6;
        if (rank.find("A") != std::string::npos) return 5;
        if (rank.find("B") != std::string::npos) return 4;
        if (rank.find("C") != std::string::npos) return 3;
        if (rank.find("D") != std::string::npos) return 2;
        if (rank.find("E") != std::string::npos) return 1;
        return 0;
    }

    std::string rankName(int index)
    {
        static const std::vector<std::string> ranks = {
            "F", "E", "D", "C", "B", "A", "S", "SS", "SSS", "Héros mondial", "Légende", "Dieu"
        };
        index = std::clamp(index, 0, static_cast<int>(ranks.size()) - 1);
        return ranks[static_cast<std::size_t>(index)];
    }

    bool hasActiveGroup(const Player& player)
    {
        // Hestia occupies a real combat slot when active, so she counts as a genuine group member too.
        return !RecruitedAllyCombatRoster::collectActive(player).empty();
    }

    int groupMaximumQuestRankIndex(int leaderRankIndex, const std::vector<int>& allyRankIndices)
    {
        std::vector<int> ranks;
        ranks.reserve(allyRankIndices.size() + 1);
        ranks.push_back(std::clamp(leaderRankIndex, 0, 11));
        for (const int rank : allyRankIndices) ranks.push_back(std::clamp(rank, 0, 11));

        if (ranks.size() == 1) return ranks.front();

        // Small parties stay individually accountable: the weakest member may only be
        // taken one rank above their own guild rank.
        if (ranks.size() < 5)
        {
            const int weakest = *std::min_element(ranks.begin(), ranks.end());
            return std::min(11, weakest + 1);
        }

        // Five or more members use the group's average rank. The average is rounded to
        // the nearest rank. When it sits at most one full rank below the strongest
        // member, the party is coherent enough to attempt average + 1.
        const int total = std::accumulate(ranks.begin(), ranks.end(), 0);
        const double average = static_cast<double>(total) / static_cast<double>(ranks.size());
        const int averageRank = std::clamp(static_cast<int>(std::lround(average)), 0, 11);
        const int strongest = *std::max_element(ranks.begin(), ranks.end());
        int maximum = averageRank;
        if (static_cast<double>(strongest) - average <= 1.0) maximum = std::min(11, averageRank + 1);
        return maximum;
    }

    int maximumQuestRankIndex(const Player& player, const std::string& guildRank, const std::string& reliabilityPellet)
    {
        const int leaderRank = rankIndex(guildRank);
        std::vector<int> allyRanks;
        const auto active = RecruitedAllyCombatRoster::collectActive(player);
        allyRanks.reserve(active.size());
        for (const RecruitedAllyCombatSupport& ally : active) allyRanks.push_back(ally.currentRankIndex);

        int maxIndex = groupMaximumQuestRankIndex(leaderRank, allyRanks);

        // Reliability sanctions remain stricter than group privileges.
        if (reliabilityPellet == "orange") maxIndex = std::min(maxIndex, rankIndex("B"));
        else if (reliabilityPellet == "rouge") maxIndex = std::min(maxIndex, rankIndex("D"));

        return std::max(0, maxIndex);
    }

    bool groupMembersCanTakeRank(int leaderRankIndex, const std::vector<int>& allyRankIndices, int questRankIndex)
    {
        return questRankIndex <= groupMaximumQuestRankIndex(leaderRankIndex, allyRankIndices);
    }

    bool canAccept(const Player& player, const Quest& quest, const std::string& guildRank, const std::string& reliabilityPellet)
    {
        if (!quest.guildQuest) return true;
        return rankIndex(quest.rank) <= maximumQuestRankIndex(player, guildRank, reliabilityPellet);
    }
}
