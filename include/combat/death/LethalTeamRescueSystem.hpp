#pragma once

#include "combat/ally/RecruitedAllyCombatRoster.hpp"

#include <string>
#include <vector>

class Player;
class Random;

struct LethalTeamRescueAssessment
{
    bool teamPresent = false;
    bool hasCapableRescuer = false;
    int survivalChancePercent = 0;
    int revivedHealthPercent = 0;
    std::string rescuerName;
    std::string rescueMethod;
};

struct LethalTeamRescueResult : public LethalTeamRescueAssessment
{
    int roll = 0;
    bool survived = false;
};

namespace LethalTeamRescueSystem
{
    LethalTeamRescueAssessment assessRecruitedTeam(
        const std::vector<RecruitedAllyCombatSupport>& activeAllies,
        const std::vector<RecruitedAllyCombatSupport>& reserveAllies
    );

    LethalTeamRescueResult tryRecruitedTeamRescue(
        Player& player,
        const std::vector<RecruitedAllyCombatSupport>& activeAllies,
        const std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        Random& random,
        const std::string& contextId
    );

    bool humanPlayerCanPerformLethalRescue(const Player& player);
    int humanPlayerRescueStrength(const Player& player);

    LethalTeamRescueResult tryHumanPartyRescue(
        Player& downedPlayer,
        const std::vector<Player*>& party,
        Random& random,
        const std::string& contextId
    );
}
