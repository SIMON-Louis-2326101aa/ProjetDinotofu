#pragma once

#include <string>
#include <vector>

class Player;

struct CrimeNetworkWarningResult
{
    int knownContactsAffected = 0;
    int localContactsAffected = 0;
    int intercityContactsAffected = 0;
    int intercityRelaysBlocked = 0;
    std::vector<std::string> affectedContactIds;
    std::vector<std::string> blockedContactIds;
};

struct CrimeNetworkRelayAssessment
{
    bool known = false;
    bool sameNetwork = false;
    bool available = false;
    bool relayLikely = false;
    int relayReadiness = 0;
    std::string reason;
};

namespace CrimeNetworkSystem
{
    // Propagates only through contacts the player has already discovered. The network catalog
    // defines a plausible channel; it never reveals an unknown contact or invents a culprit.
    CrimeNetworkWarningResult propagateKnownWarning(
        Player& player,
        const std::string& sourceContactId,
        int localPressureGain,
        int intercityPressureGain,
        int cooperationLoss,
        const std::string& reason
    );

    // Inter-city relays are not telephones: detention, flight, pressure, cooperation with the
    // player and repeated same-day warnings can weaken or cut a known relay.
    CrimeNetworkRelayAssessment assessKnownRelay(
        const Player& player,
        const std::string& sourceContactId,
        const std::string& targetContactId
    );
}
