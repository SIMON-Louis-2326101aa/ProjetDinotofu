#pragma once

#include "combat/ally/RecruitedAllyCombatRoster.hpp"

#include <string>
#include <vector>

class Player;

struct EmployerTeamMember
{
    std::string memberId;
    std::string name;
    std::string race;
    std::string job;
    std::string trait;
    int level = 1;
    int rankIndex = 0;
    int maxHp = 1;
    int currentHp = 1;
    int weaponQuality = 1;
    int armorQuality = 1;
    int supportKitQuality = 0;
    int mood = 50;
    int loyalty = 50;
    bool leader = false;
    bool available = true;
};

struct EmployerTeamFormation
{
    bool playerPresent = true;
    bool playerInReserve = false;
    std::vector<EmployerTeamMember> activeMembers;
    std::vector<EmployerTeamMember> reserveMembers;
    std::vector<EmployerTeamMember> unavailableMembers;
};

namespace EmployerTeamRosterSystem
{
    inline constexpr int MaxNpcMembersWhilePlayerEmployed = 5;

    // Creates the roster once for the currently active external contract.
    // Existing records are never regenerated, so identities stay stable across save/reload.
    bool ensureRoster(Player& player);

    std::vector<EmployerTeamMember> rosterFor(const Player& player);
    EmployerTeamFormation formationFor(const Player& player, bool playerPresent = true);

    // Ordre d'ouverture des places principales. Si le chef est opérationnel, il joue toujours en premier.
    std::vector<std::string> openingTurnOrder(const Player& player, bool playerPresent = true);

    RecruitedAllyCombatSupport toCombatSupport(const EmployerTeamMember& member);

    bool setMemberVitals(Player& player, const std::string& memberId, int currentHp, bool available, const std::string& status = "operationnel");
    bool recordMemberCombatProgress(Player& player, const std::string& memberId, int participationPoints);

    std::string statusLabel(const EmployerTeamMember& member);
};
