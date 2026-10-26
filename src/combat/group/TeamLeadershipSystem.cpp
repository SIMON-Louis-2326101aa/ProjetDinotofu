#include "combat/group/TeamLeadershipSystem.hpp"

#include "entity/Player.hpp"

#include <algorithm>
#include <cstdlib>

namespace
{
    constexpr const char* Category = "team_leadership_state";

    std::string stateLabel(const Player& player, const std::string& key, const std::string& fallback)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == Category && record.key == key && !record.label.empty())
            {
                return record.label;
            }
        }
        return fallback;
    }

    int stateInt(const Player& player, const std::string& key, int fallback)
    {
        const std::string value = stateLabel(player, key, std::to_string(fallback));
        try
        {
            return std::max(0, std::stoi(value));
        }
        catch (...)
        {
            return std::max(0, fallback);
        }
    }

    std::string encodeEmpty(const std::string& value)
    {
        return value.empty() ? "-" : value;
    }

    std::string decodeEmpty(const std::string& value)
    {
        return value == "-" ? "" : value;
    }

    void setState(Player& player, const std::string& key, const std::string& value)
    {
        player.recordCanonicalEvent(Category, key, encodeEmpty(value), 1);
    }
}

TeamLeadershipContext TeamLeadershipSystem::contextFor(const Player& player)
{
    TeamLeadershipContext context;
    const std::string mode = stateLabel(player, "mode", "player_led");

    if (mode == "external_npc")
    {
        context.leaderKind = TeamLeaderKind::ExternalNpc;
        context.playerIsLeader = false;
        context.playerIsPaidMember = true;
        context.playerPaysTeamCosts = false;
        context.canIssueTacticalOrders = false;
        context.canArrangePassageOrder = false;
    }
    else if (mode == "other_player")
    {
        context.leaderKind = TeamLeaderKind::OtherPlayer;
        context.playerIsLeader = false;
        context.playerIsPaidMember = stateInt(player, "salary", 0) > 0;
        context.playerPaysTeamCosts = false;
        context.canIssueTacticalOrders = false;
        context.canArrangePassageOrder = false;
    }
    else
    {
        context.leaderKind = TeamLeaderKind::PlayerCharacter;
        context.playerIsLeader = true;
        context.playerIsPaidMember = false;
        context.playerPaysTeamCosts = true;
        context.canIssueTacticalOrders = true;
        context.canArrangePassageOrder = true;
    }

    context.leaderName = decodeEmpty(stateLabel(player, "leader_name", context.playerIsLeader ? player.getName() : "-"));
    if (context.leaderName.empty() && context.playerIsLeader) context.leaderName = player.getName();
    context.organizationId = decodeEmpty(stateLabel(player, "organization_id", "-"));
    context.organizationName = decodeEmpty(stateLabel(player, "organization_name", "-"));
    context.salaryEconomyUnits = context.playerIsLeader ? 0 : stateInt(player, "salary", 0);
    return context;
}

void TeamLeadershipSystem::setPersonalTeam(Player& player)
{
    setState(player, "mode", "player_led");
    setState(player, "leader_name", player.getName());
    setState(player, "organization_id", "");
    setState(player, "organization_name", "");
    setState(player, "salary", "0");
    player.recordCanonicalEvent(
        "team_membership_history",
        "personal_team_return",
        "Retour sous sa propre direction d'équipe.",
        1
    );
}

void TeamLeadershipSystem::setExternalNpcTeam(
    Player& player,
    const std::string& leaderName,
    const std::string& organizationId,
    const std::string& organizationName,
    int salaryEconomyUnits)
{
    setState(player, "mode", "external_npc");
    setState(player, "leader_name", leaderName.empty() ? "Chef d'équipe" : leaderName);
    setState(player, "organization_id", organizationId);
    setState(player, "organization_name", organizationName);
    setState(player, "salary", std::to_string(std::max(0, salaryEconomyUnits)));
    player.recordCanonicalEvent(
        "team_membership_history",
        !organizationId.empty() ? organizationId : (leaderName.empty() ? "external_team" : leaderName),
        "Membre salarié sous la direction de " + (leaderName.empty() ? std::string("un chef d'équipe") : leaderName)
            + (organizationName.empty() ? "." : " — " + organizationName + "."),
        1
    );
}

void TeamLeadershipSystem::setOtherPlayerLedTeam(
    Player& player,
    const std::string& leaderName,
    int salaryEconomyUnits)
{
    setState(player, "mode", "other_player");
    setState(player, "leader_name", leaderName.empty() ? "Autre joueur" : leaderName);
    setState(player, "organization_id", "");
    setState(player, "organization_name", "");
    setState(player, "salary", std::to_string(std::max(0, salaryEconomyUnits)));
}

bool TeamLeadershipSystem::playerCanIssueTacticalOrders(const Player& player)
{
    return contextFor(player).canIssueTacticalOrders;
}

bool TeamLeadershipSystem::playerCanArrangePassageOrder(const Player& player)
{
    return contextFor(player).canArrangePassageOrder;
}

bool TeamLeadershipSystem::playerReceivesTeamSalary(const Player& player)
{
    return contextFor(player).playerIsPaidMember;
}

bool TeamLeadershipSystem::playerPaysTeamCosts(const Player& player)
{
    return contextFor(player).playerPaysTeamCosts;
}

std::string TeamLeadershipSystem::leaderDisplayName(const Player& player)
{
    const TeamLeadershipContext context = contextFor(player);
    if (!context.leaderName.empty()) return context.leaderName;
    return context.playerIsLeader ? player.getName() : "Chef d'équipe";
}
