#pragma once

#include <string>
#include <vector>

class Player;

struct CrimeLocalOpportunityEntry
{
    std::string eventId;
    std::string cityId;
    std::string kind;
    std::string sourceId;
    std::string title;
    std::string client;
    std::string objective;
    int eventDay = 0;
    std::vector<std::string> lines;
};

struct CrimeLocalOpportunityAcceptResult
{
    bool accepted = false;
    std::string questId;
    std::vector<std::string> lines;
};

namespace CrimeLocalOpportunitySystem
{
    bool queue(
        Player& player,
        const std::string& cityId,
        const std::string& kind,
        const std::string& sourceId,
        const std::string& reason
    );

    std::vector<CrimeLocalOpportunityEntry> pending(const Player& player, const std::string& cityId = "");
    int pendingCount(const Player& player, const std::string& cityId = "");

    CrimeLocalOpportunityAcceptResult accept(Player& player, const std::string& eventId);
}
