#pragma once

#include <string>

class Player;

enum class TeamLeaderKind
{
    PlayerCharacter,
    ExternalNpc,
    OtherPlayer
};

struct TeamLeadershipContext
{
    TeamLeaderKind leaderKind = TeamLeaderKind::PlayerCharacter;
    std::string leaderName;
    std::string organizationId;
    std::string organizationName;
    int salaryEconomyUnits = 0;
    bool playerIsLeader = true;
    bool playerIsPaidMember = false;
    bool playerPaysTeamCosts = true;
    bool canIssueTacticalOrders = true;
    bool canArrangePassageOrder = true;
};

namespace TeamLeadershipSystem
{
    TeamLeadershipContext contextFor(const Player& player);

    void setPersonalTeam(Player& player);
    void setExternalNpcTeam(
        Player& player,
        const std::string& leaderName,
        const std::string& organizationId,
        const std::string& organizationName,
        int salaryEconomyUnits
    );
    void setOtherPlayerLedTeam(
        Player& player,
        const std::string& leaderName,
        int salaryEconomyUnits = 0
    );

    bool playerCanIssueTacticalOrders(const Player& player);
    bool playerCanArrangePassageOrder(const Player& player);
    bool playerReceivesTeamSalary(const Player& player);
    bool playerPaysTeamCosts(const Player& player);
    std::string leaderDisplayName(const Player& player);
};
