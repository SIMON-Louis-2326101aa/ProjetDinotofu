#pragma once

#include <string>
#include <vector>

class Player;
struct Quest;

enum class BountyResolutionKind
{
    CapturedBySurrender,
    CapturedAfterCombat,
    KilledInCombat
};

struct BountyRegionalState
{
    int aliveCaptures = 0;
    int lethalResolutions = 0;
    int noisyHunts = 0;
    int recordedEscapeAttempts = 0;

    int awarenessPressure = 0;
    int supportShift = 0;
    int resourceShift = 0;
    int mobilityPressure = 0;
};

struct BountyRegionalAdaptationResult
{
    bool applied = false;
    int awarenessDelta = 0;
    int localSupportDelta = 0;
    int resourcesDelta = 0;
    int mobilityDelta = 0;
    std::vector<std::string> lines;
};

struct BountyRegionalConsequenceResult
{
    bool applied = false;
    int localReputationGain = 0;
    int knownCrimeContactsAffected = 0;
    bool noisyHunt = false;
    std::vector<std::string> lines;
};

namespace BountyRegionalConsequenceSystem
{
    BountyRegionalState stateFor(const Player& player, const std::string& cityId);

    // Applies regional history when a generated target enters a city. Re-reading the same city is
    // idempotent; relocating to another city applies only the environmental delta plus that new
    // region's existing bounty history.
    BountyRegionalAdaptationResult adaptTargetToRegion(Player& player, Quest& quest);

    // Records the final local consequences exactly once. Only already-known criminal contacts
    // can react directly; this system never reveals an unknown contact or network to the player.
    BountyRegionalConsequenceResult applyResolution(
        Player& player,
        Quest& quest,
        BountyResolutionKind resolution
    );

    std::string resolutionId(BountyResolutionKind resolution);
    std::string resolutionLabel(BountyResolutionKind resolution);
}
