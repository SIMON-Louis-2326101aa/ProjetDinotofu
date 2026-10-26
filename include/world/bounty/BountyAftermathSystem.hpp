#pragma once

#include <string>
#include <vector>

class Player;
struct Quest;
enum class BountyResolutionKind;

enum class BountyAftermathActorType
{
    Issuer,
    Organization,
    CloseRelation
};

enum class BountyAftermathResponse
{
    Professional,
    Conciliatory,
    Firm,
    OfferHelp,
    EndConversation
};

struct BountyAftermathEntry
{
    std::string eventId;
    BountyAftermathActorType actorType = BountyAftermathActorType::Issuer;
    std::string actorId;
    std::string actorName;
    std::string targetId;
    std::string targetName;
    std::string relationType;
    std::string resolutionId;
    int eventDay = 0;
    bool targetAlive = true;
    bool preferredOutcome = true;
    std::vector<std::string> lines;
};

struct BountyAftermathResponseOption
{
    BountyAftermathResponse response = BountyAftermathResponse::Professional;
    std::string label;
    std::string hint;
};

struct BountyAftermathResolutionResult
{
    bool resolved = false;
    std::vector<std::string> lines;
};

namespace BountyAftermathSystem
{
    // Creates persistent follow-ups from a named bounty. These are correspondence / meetings to
    // resolve later at the guild; they do not teleport relatives or organizations to the player.
    int queueResolution(Player& player, const Quest& quest, BountyResolutionKind resolution);

    std::vector<BountyAftermathEntry> pending(const Player& player);
    int pendingCount(const Player& player);

    std::vector<BountyAftermathResponseOption> responseOptions(const BountyAftermathEntry& entry);
    BountyAftermathResolutionResult respond(
        Player& player,
        const std::string& eventId,
        BountyAftermathResponse response
    );

    std::string actorTypeLabel(BountyAftermathActorType actorType);
}
