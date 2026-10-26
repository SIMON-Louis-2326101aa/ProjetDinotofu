#include "combat/group/TeamEmploymentSystem.hpp"

#include "combat/group/EmployerTeamRosterSystem.hpp"
#include "combat/group/EmployerTeamDisciplineSystem.hpp"
#include "combat/group/TeamLeadershipSystem.hpp"
#include "entity/Player.hpp"
#include "quest/Quest.hpp"
#include "quest/QuestLog.hpp"

#include <algorithm>
#include <limits>

namespace
{
    constexpr const char* StateCategory = "team_employment_state";

    std::string stateLabel(const Player& player, const std::string& key, const std::string& fallback = "")
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == StateCategory && record.key == key)
            {
                return record.label.empty() ? fallback : record.label;
            }
        }
        return fallback;
    }

    int stateInt(const Player& player, const std::string& key, int fallback)
    {
        try
        {
            return std::stoi(stateLabel(player, key, std::to_string(fallback)));
        }
        catch (...)
        {
            return fallback;
        }
    }

    void setState(Player& player, const std::string& key, const std::string& value)
    {
        player.recordCanonicalEvent(StateCategory, key, value.empty() ? "-" : value, 1);
    }

    std::string decodeEmpty(const std::string& value)
    {
        return value == "-" ? "" : value;
    }

    int contractSalaryCutoffDay(const TeamEmploymentContract& contract, int currentDay)
    {
        if (contract.plannedEndDay >= 0)
        {
            return std::min(currentDay, contract.plannedEndDay);
        }
        return currentDay;
    }

    TeamSalarySettlement settleDueSalary(Player& player, int retentionPercent)
    {
        TeamSalarySettlement settlement;
        TeamEmploymentContract contract = TeamEmploymentSystem::contractFor(player);
        settlement.previousSettledDay = contract.lastSettledDay;
        settlement.newSettledDay = contract.lastSettledDay;
        settlement.retentionPercent = std::clamp(retentionPercent, 0, 75);

        const int periods = TeamEmploymentSystem::dueSalaryPeriods(player);
        if (!contract.active || periods <= 0 || contract.salaryEconomyUnits <= 0)
        {
            return settlement;
        }

        const long long gross = static_cast<long long>(periods) * static_cast<long long>(contract.salaryEconomyUnits);
        const long long cappedGross = std::min<long long>(gross, std::numeric_limits<int>::max());
        settlement.economyUnitsRetained = cappedGross * settlement.retentionPercent / 100;
        settlement.economyUnitsPaid = std::max<long long>(0, cappedGross - settlement.economyUnitsRetained);
        if (settlement.economyUnitsPaid > 0)
        {
            player.getInventory().earnEconomyUnits(settlement.economyUnitsPaid);
            player.refreshCurrencyTitles();
        }

        settlement.paid = settlement.economyUnitsPaid > 0;
        settlement.periodsPaid = periods;
        settlement.newSettledDay = contract.lastSettledDay + periods * contract.payIntervalDays;

        setState(player, "last_settled_day", std::to_string(settlement.newSettledDay));
        setState(player, "salary_periods_paid", std::to_string(contract.salaryPeriodsPaid + periods));
        player.recordCanonicalEvent(
            "team_employment_payments",
            contract.contractId.empty() ? "external_team" : contract.contractId,
            (contract.organizationName.empty() ? std::string("Employeur") : contract.organizationName)
                + " | " + std::to_string(periods) + " période(s) réglée(s)"
                + " | versé=" + std::to_string(settlement.economyUnitsPaid)
                + " | retenu=" + std::to_string(settlement.economyUnitsRetained)
                + " | retenue_pct=" + std::to_string(settlement.retentionPercent)
                + " | jusqu'au jour=" + std::to_string(settlement.newSettledDay),
            static_cast<int>(std::min<long long>(std::max<long long>(1, cappedGross), std::numeric_limits<int>::max()))
        );
        return settlement;
    }

    bool closeContract(Player& player, const std::string& status, const std::string& reason, int salaryRetentionPercent = 0)
    {
        TeamEmploymentContract contract = TeamEmploymentSystem::contractFor(player);
        if (!contract.active)
        {
            return false;
        }

        const std::string missionPrefix = "team_job_" + contract.contractId + "_";
        for (Quest& quest : player.getQuestLog().getQuests())
        {
            if (contract.contractId.empty() || quest.id.rfind(missionPrefix, 0) != 0
                || !quest.accepted || quest.turnedIn || quest.failed)
            {
                continue;
            }

            quest.failed = true;
            quest.failureReason = "Ordre employeur interrompu : le contrat avec cette équipe a pris fin.";
            player.recordCanonicalEvent(
                "team_employment_missions_failed",
                quest.id,
                (contract.organizationName.empty() ? std::string("Employeur") : contract.organizationName)
                    + " | mission interrompue par la fin du contrat : " + (quest.title.empty() ? quest.id : quest.title),
                1
            );
        }

        const TeamSalarySettlement finalSettlement = settleDueSalary(player, salaryRetentionPercent);
        const int endDay = std::max(contract.startDay, player.getWorldDaysElapsed());
        setState(player, "status", status);
        setState(player, "end_day", std::to_string(endDay));
        setState(player, "end_reason", reason.empty() ? status : reason);

        player.recordCanonicalEvent(
            "team_employment_history",
            contract.contractId.empty() ? (contract.organizationId.empty() ? "external_team" : contract.organizationId) : contract.contractId,
            (contract.organizationName.empty() ? std::string("Équipe externe") : contract.organizationName)
                + " | fin jour=" + std::to_string(endDay)
                + " | raison=" + (reason.empty() ? status : reason)
                + " | retenue_salaire=" + std::to_string(finalSettlement.retentionPercent) + "%"
                + " | retenu=" + std::to_string(finalSettlement.economyUnitsRetained),
            1
        );

        TeamLeadershipSystem::setPersonalTeam(player);
        return true;
    }
}

TeamEmploymentContract TeamEmploymentSystem::contractFor(const Player& player)
{
    TeamEmploymentContract contract;
    contract.active = stateLabel(player, "status", "none") == "active";
    contract.contractId = decodeEmpty(stateLabel(player, "contract_id", "-"));
    contract.leaderName = decodeEmpty(stateLabel(player, "leader_name", "-"));
    contract.organizationId = decodeEmpty(stateLabel(player, "organization_id", "-"));
    contract.organizationName = decodeEmpty(stateLabel(player, "organization_name", "-"));
    contract.roleName = decodeEmpty(stateLabel(player, "role_name", "-"));
    contract.startDay = std::max(0, stateInt(player, "start_day", player.getWorldDaysElapsed()));
    contract.plannedEndDay = stateInt(player, "planned_end_day", -1);
    contract.salaryEconomyUnits = std::max(0, stateInt(player, "salary", 0));
    contract.payIntervalDays = std::max(1, stateInt(player, "pay_interval_days", 7));
    contract.lastSettledDay = std::max(contract.startDay, stateInt(player, "last_settled_day", contract.startDay));
    contract.salaryPeriodsPaid = std::max(0, stateInt(player, "salary_periods_paid", 0));
    return contract;
}

bool TeamEmploymentSystem::hasActiveContract(const Player& player)
{
    return contractFor(player).active;
}

bool TeamEmploymentSystem::startExternalContract(
    Player& player,
    const std::string& contractId,
    const std::string& leaderName,
    const std::string& organizationId,
    const std::string& organizationName,
    const std::string& roleName,
    int salaryEconomyUnits,
    int payIntervalDays,
    int durationDays)
{
    if (hasActiveContract(player) || contractId.empty() || leaderName.empty() || salaryEconomyUnits <= 0 || payIntervalDays <= 0)
    {
        return false;
    }

    const int startDay = std::max(0, player.getWorldDaysElapsed());
    const int plannedEndDay = durationDays > 0 ? startDay + durationDays : -1;

    setState(player, "status", "active");
    setState(player, "contract_id", contractId);
    setState(player, "leader_name", leaderName);
    setState(player, "organization_id", organizationId);
    setState(player, "organization_name", organizationName);
    setState(player, "role_name", roleName);
    setState(player, "start_day", std::to_string(startDay));
    setState(player, "planned_end_day", std::to_string(plannedEndDay));
    setState(player, "salary", std::to_string(std::max(0, salaryEconomyUnits)));
    setState(player, "pay_interval_days", std::to_string(std::max(1, payIntervalDays)));
    setState(player, "last_settled_day", std::to_string(startDay));
    setState(player, "salary_periods_paid", "0");
    setState(player, "end_day", "-1");
    setState(player, "end_reason", "-");

    TeamLeadershipSystem::setExternalNpcTeam(
        player,
        leaderName,
        organizationId,
        organizationName,
        salaryEconomyUnits
    );

    // L'équipe employeuse possède ses propres collègues persistants.
    // Ils ne deviennent jamais des recrues personnelles du joueur.
    EmployerTeamRosterSystem::ensureRoster(player);
    EmployerTeamDisciplineSystem::initializeForContract(player);

    player.recordCanonicalEvent(
        "team_employment_history",
        contractId,
        (organizationName.empty() ? std::string("Équipe externe") : organizationName)
            + " | chef=" + leaderName
            + " | rôle=" + (roleName.empty() ? std::string("membre") : roleName)
            + " | début jour=" + std::to_string(startDay)
            + " | paie=" + std::to_string(salaryEconomyUnits)
            + " unités/" + std::to_string(payIntervalDays) + "j",
        1
    );
    return true;
}

int TeamEmploymentSystem::dueSalaryPeriods(const Player& player)
{
    const TeamEmploymentContract contract = contractFor(player);
    if (!contract.active || contract.salaryEconomyUnits <= 0 || contract.payIntervalDays <= 0)
    {
        return 0;
    }

    const int cutoffDay = contractSalaryCutoffDay(contract, std::max(0, player.getWorldDaysElapsed()));
    if (cutoffDay <= contract.lastSettledDay)
    {
        return 0;
    }
    return std::max(0, (cutoffDay - contract.lastSettledDay) / contract.payIntervalDays);
}

long long TeamEmploymentSystem::dueSalaryEconomyUnits(const Player& player)
{
    const TeamEmploymentContract contract = contractFor(player);
    return static_cast<long long>(dueSalaryPeriods(player)) * static_cast<long long>(contract.salaryEconomyUnits);
}

TeamSalarySettlement TeamEmploymentSystem::collectDueSalary(Player& player)
{
    return settleDueSalary(player, 0);
}

bool TeamEmploymentSystem::resign(Player& player, const std::string& reason)
{
    return closeContract(player, "resigned", reason);
}

bool TeamEmploymentSystem::dismiss(Player& player, const std::string& reason)
{
    return closeContract(player, "dismissed", reason);
}

bool TeamEmploymentSystem::dismissForDiscipline(Player& player, const std::string& reason, int salaryRetentionPercent)
{
    return closeContract(
        player,
        "dismissed_disciplinary",
        reason.empty() ? "faute_professionnelle" : reason,
        std::clamp(salaryRetentionPercent, 0, EmployerTeamDisciplineSystem::MaxDisciplinarySalaryRetentionPercent)
    );
}

bool TeamEmploymentSystem::finishContract(Player& player, const std::string& reason)
{
    return closeContract(player, "completed", reason);
}

bool TeamEmploymentSystem::finalizeIfExpired(Player& player)
{
    const TeamEmploymentContract contract = contractFor(player);
    if (!contract.active || contract.plannedEndDay < 0 || player.getWorldDaysElapsed() < contract.plannedEndDay)
    {
        return false;
    }
    return finishContract(player, "echeance_atteinte");
}

void TeamEmploymentSystem::recordEmployerMissionCompleted(Player& player, const std::string& questId, const std::string& title)
{
    const TeamEmploymentContract contract = contractFor(player);
    if (!contract.active || questId.empty())
    {
        return;
    }
    player.recordCanonicalEvent(
        "team_employment_missions_completed",
        questId,
        (contract.organizationName.empty() ? std::string("Employeur") : contract.organizationName)
            + " | mission réussie : " + (title.empty() ? questId : title),
        1
    );
    EmployerTeamDisciplineSystem::recordMissionCompleted(player, questId);
}

void TeamEmploymentSystem::recordEmployerMissionFailed(Player& player, const std::string& questId, const std::string& title)
{
    const TeamEmploymentContract contract = contractFor(player);
    if (!contract.active || questId.empty())
    {
        return;
    }
    player.recordCanonicalEvent(
        "team_employment_missions_failed",
        questId,
        (contract.organizationName.empty() ? std::string("Employeur") : contract.organizationName)
            + " | mission échouée : " + (title.empty() ? questId : title),
        1
    );
    EmployerTeamDisciplineSystem::recordMissionFailed(player, questId, "mission_employeur_echouee");
}
