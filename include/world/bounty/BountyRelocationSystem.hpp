#pragma once

#include <string>
#include <vector>

class Player;
struct Quest;

struct BountyRelocationResult
{
    bool evaluated = false;
    bool moved = false;
    int relocationChancePercent = 0;
    bool usedTradeRoute = false;
    std::string tradeTransitKind;
    std::string fromCityId;
    std::string toCityId;
    std::string toBiome;
    std::vector<std::string> lines;
};

struct BountyTransitTrailUseResult
{
    bool evaluated = false;
    bool useful = false;
    int cluesGained = 0;
    int noiseAdded = 0;
    std::vector<std::string> lines;
};

namespace BountyRelocationSystem
{
    // Ensures a targeted bounty has a persistent city + local refuge. Legacy quests first try to
    // infer their city from the authored biome before falling back to the supplied city.
    void ensureLocalPosition(Quest& quest, const std::string& fallbackCityId);

    // Some mobile/experienced targets can abandon a city after a real failed interception.
    // Destinations are limited to cities the player can actually reach, so a live contract cannot
    // silently move behind an unavailable level/boss gate.
    BountyRelocationResult tryRelocateAfterEscape(
        const Player& player,
        Quest& quest,
        int roll1To100
    );

    bool isTargetCurrentlyInCity(const Quest& quest, const std::string& cityId);
    std::string locationSummary(const Quest& quest);
    std::string transitTrailSummary(const Quest& quest);

    // Uses the one-shot inter-city trace during a later local investigation. A good trace helps,
    // but it never grants an immediate interception by itself.
    BountyTransitTrailUseResult useTransitTrail(Quest& quest, int investigatorBias, int roll1To100);
}
