#pragma once

#include <string>

class Random;

struct CombatLeaderRotationContext
{
    bool leaderPresent = false;
    int activeCount = 0;
    int reserveCount = 0;
    int lowHpActiveCount = 0;
    int criticalHpActiveCount = 0;
    bool playerWaiting = false;
    int leaderIntelligence = 50;
    int groupSolidarity = 50;
    bool abandonsWounded = false;
    int roundNumber = 1;
};

struct CombatLeaderRotationDecision
{
    bool attempt = false;
    int swapCount = 0;
    int chancePercent = 0;
    std::string reason;
};

namespace CombatLeaderRotationSystem
{
    // Décision bornée : une rotation n'est jamais automatique, même avec un excellent chef.
    CombatLeaderRotationDecision decide(const CombatLeaderRotationContext& context, Random& random);

    // Un membre très bas PV peut rarement refuser un ordre de rotation. Hors bas PV : 0 %.
    int refusalChancePercent(int hpPercent, int loyaltyOrCohesion = 50, bool stubborn = false);
    bool refusesRotation(int hpPercent, int loyaltyOrCohesion, bool stubborn, Random& random);
}
