#pragma once

#include <string>
#include <vector>

class Player;
struct PlayerVaultTheftCase;

struct CrimeJusticeAssessment
{
    bool knownContact = false;
    bool enoughEvidenceForDurableArrest = false;
    int evidenceScore = 0;
    std::string legalBasis;
    std::vector<std::string> lines;
};

struct CrimeJusticeResolutionResult
{
    bool changed = false;
    bool durableArrestApplied = false;
    std::vector<std::string> lines;
};

namespace CrimeJusticeSystem
{
    CrimeJusticeAssessment assessVaultTheftContact(
        const Player& player,
        const PlayerVaultTheftCase& theftCase,
        const std::string& contactId
    );

    CrimeJusticeResolutionResult tryApplyDurableArrest(
        Player& player,
        const PlayerVaultTheftCase& theftCase,
        const std::string& contactId
    );
}
