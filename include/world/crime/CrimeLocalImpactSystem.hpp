#pragma once

#include "world/CityEconomyDynamicsSystem.hpp"

#include <string>
#include <vector>

class Player;

struct CrimeLocalImpactSnapshot
{
    bool observable = false;
    int pressureScore = 0;
    int knownContacts = 0;
    int arrestedContacts = 0;
    int unavailableContacts = 0;
    int knownNetworks = 0;
    int competitionTension = 0;
    int recentDisruptionWeight = 0;
    int recentStabilizationWeight = 0;
    int quietRecoveryDays = 0;
    int serviceQualityDelta = 0;
    int tradeSellModifierPercent = 0;
    int transportRiskPercent = 0;
    std::string levelId;
    std::string headline;
    std::vector<std::string> lines;
};

struct CrimeLocalImpactRegistration
{
    bool recorded = false;
    bool rumorRecorded = false;
    bool opportunityQueued = false;
    std::string opportunityKind;
    std::vector<std::string> lines;
};

namespace CrimeLocalImpactSystem
{
    CrimeLocalImpactSnapshot assess(const Player& player, const std::string& cityId);

    CrimeLocalImpactRegistration registerDisruption(
        Player& player,
        const std::string& cityId,
        const std::string& sourceId,
        int severity,
        const std::string& reason
    );

    void applyEconomicPressure(
        const Player& player,
        const std::string& cityId,
        CityEconomicPulse& pulse
    );

    std::vector<std::string> informationLines(const Player& player, const std::string& cityId);
}
