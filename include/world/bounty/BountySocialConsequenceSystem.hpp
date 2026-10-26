#pragma once

#include <string>
#include <vector>

class Player;
struct Quest;
enum class BountyResolutionKind;

struct BountySocialConsequenceResult
{
    bool applied = false;
    bool issuerSatisfied = false;
    bool closeRelationReacted = false;
    int knownNetworkContactsAffected = 0;
    std::vector<std::string> lines;
};

namespace BountySocialConsequenceSystem
{
    BountySocialConsequenceResult applyResolution(
        Player& player,
        const Quest& quest,
        BountyResolutionKind resolution
    );
}
