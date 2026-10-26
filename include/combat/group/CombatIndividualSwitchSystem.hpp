#pragma once

#include <string>

class Random;

struct CombatIndividualSwitchContext
{
    bool reserveAvailable = false;
    bool leaderPresent = false;
    bool memberIsLeader = false;
    int hpPercent = 100;
    int fearPercent = 0;
    int dangerPercent = 50;
    int intelligence = 50;
    int discipline = 50;
    int loyaltyOrCohesion = 50;
    bool stubborn = false;
    bool survivalUrgent = false;
    bool tacticalOpportunity = false;
};

struct CombatIndividualSwitchDecision
{
    bool wantsSwitch = false;
    bool asksPermission = false;
    bool actsWithoutPermission = false;
    int desireChancePercent = 0;
    int bypassChancePercent = 0;
    std::string reason;
};

struct CombatSwitchAuthorizationContext
{
    int leaderIntelligence = 50;
    int leaderSolidarity = 50;
    int outgoingHpPercent = 100;
    int incomingHpPercent = 100;
    int tacticalNeedPercent = 50;
    bool outgoingCritical = false;
    bool incomingFresher = false;
};

namespace CombatIndividualSwitchSystem
{
    CombatIndividualSwitchDecision decide(
        const CombatIndividualSwitchContext& context,
        Random& random
    );

    int authorizationChancePercent(const CombatSwitchAuthorizationContext& context);
    bool leaderAuthorizes(const CombatSwitchAuthorizationContext& context, Random& random);
}
