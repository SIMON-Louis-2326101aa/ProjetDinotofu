#pragma once

#include <string>
#include <vector>

class Player;

struct CrimeNetworkCompetitionAssessment
{
    bool observable = false;
    int knownNetworks = 0;
    int knownContacts = 0;
    int tension = 0;
    std::vector<std::string> networkIds;
    std::vector<std::string> lines;
};

struct CrimeNetworkDisruptionResult
{
    bool applied = false;
    int rivalContactsAffected = 0;
    int opportunityContacts = 0;
    std::vector<std::string> affectedContactIds;
    std::vector<std::string> lines;
};

namespace CrimeNetworkCompetitionSystem
{
    // Only known contacts participate in visible competition. Unknown rivals remain unknown.
    CrimeNetworkCompetitionAssessment assessKnownCity(const Player& player, const std::string& cityId);

    CrimeNetworkDisruptionResult applyKnownDisruption(
        Player& player,
        const std::string& sourceContactId,
        int severity,
        const std::string& reason
    );
}
