#pragma once

#include <string>
#include <vector>

class Player;

enum class WorldRumorSourceType
{
    Tavern,
    Traveler,
    Guild,
    Guard,
    Archive,
    Witness,
    DirectObservation,
    Unknown
};

struct WorldRumorStatus
{
    bool known = false;
    bool confirmed = false;
    bool contradicted = false;
    std::string rumorId;
    std::string subjectId;
    std::string text;
    std::string lastSource;
    int credibility = 0;
    int evidenceLevel = 0;
    int heardCount = 0;
    int lastDay = -1;
};

namespace WorldRumorSystem
{
    // Generic rumor/legend layer for the whole world. Factions, cities, monsters and named people
    // can all use it; REAPERS-13 is deliberately only one possible subject among many.
    std::string sourceTypeId(WorldRumorSourceType sourceType);
    int sourceCredibilityBonus(WorldRumorSourceType sourceType);

    void recordRumor(
        Player& player,
        const std::string& rumorId,
        const std::string& subjectId,
        const std::string& text,
        WorldRumorSourceType sourceType,
        int credibility,
        int evidenceLevel = 0
    );

    void confirmRumor(
        Player& player,
        const std::string& rumorId,
        const std::string& subjectId,
        const std::string& evidenceText,
        int evidenceLevel = 3
    );

    void contradictRumor(
        Player& player,
        const std::string& rumorId,
        const std::string& subjectId,
        const std::string& evidenceText
    );

    WorldRumorStatus statusFor(const Player& player, const std::string& rumorId);
    std::vector<std::string> buildSummaryLines(const Player& player, const std::string& rumorId);
}
