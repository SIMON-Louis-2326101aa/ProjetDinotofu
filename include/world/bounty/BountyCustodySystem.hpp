#pragma once

#include <string>
#include <vector>

class Player;
struct Quest;
enum class BountyResolutionKind;

enum class BountyCustodyApproach
{
    CalmDebrief,
    AskAboutNetwork,
    OfferFormalCooperation
};

struct BountyCustodyEntry
{
    std::string eventId;
    std::string targetId;
    std::string targetName;
    std::string resolutionId;
    std::string temperament;
    std::string networkId;
    int captureDay = 0;
    std::vector<std::string> lines;
};

struct BountyCustodyOption
{
    BountyCustodyApproach approach = BountyCustodyApproach::CalmDebrief;
    std::string label;
    std::string hint;
};

struct BountyCustodyResult
{
    bool resolved = false;
    bool cooperated = false;
    bool testimonyRecorded = false;
    int knownNetworkContactsAffected = 0;
    std::vector<std::string> lines;
};

namespace BountyCustodySystem
{
    // A live capture can create one optional interview at the guild. Testimony is never treated as
    // confirmed truth and cannot reveal criminal contacts the player has never discovered.
    bool queueCapture(Player& player, const Quest& quest, BountyResolutionKind resolution);

    std::vector<BountyCustodyEntry> pending(const Player& player);
    int pendingCount(const Player& player);
    std::vector<BountyCustodyOption> options(const BountyCustodyEntry& entry);

    BountyCustodyResult resolve(
        Player& player,
        const std::string& eventId,
        BountyCustodyApproach approach
    );
}
