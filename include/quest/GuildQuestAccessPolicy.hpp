#pragma once

#include <string>
#include <vector>

class Player;
struct Quest;

namespace GuildQuestAccessPolicy
{
    int rankIndex(const std::string& rank);
    std::string rankName(int index);
    bool hasActiveGroup(const Player& player);
    int maximumQuestRankIndex(const Player& player, const std::string& guildRank, const std::string& reliabilityPellet);
    bool canAccept(const Player& player, const Quest& quest, const std::string& guildRank, const std::string& reliabilityPellet);
    int groupMaximumQuestRankIndex(int leaderRankIndex, const std::vector<int>& allyRankIndices);
    bool groupMembersCanTakeRank(int leaderRankIndex, const std::vector<int>& allyRankIndices, int questRankIndex);
}
