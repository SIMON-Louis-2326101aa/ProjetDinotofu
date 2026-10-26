#pragma once

#include <string>

class Player;

struct TeamEmploymentContract
{
    bool active = false;
    std::string contractId;
    std::string leaderName;
    std::string organizationId;
    std::string organizationName;
    int startDay = 0;
    int plannedEndDay = -1;
    int salaryEconomyUnits = 0;
    int payIntervalDays = 7;
    int lastSettledDay = 0;
    int salaryPeriodsPaid = 0;
    std::string roleName;
};

struct TeamSalarySettlement
{
    bool paid = false;
    int periodsPaid = 0;
    long long economyUnitsPaid = 0;
    long long economyUnitsRetained = 0;
    int retentionPercent = 0;
    int previousSettledDay = 0;
    int newSettledDay = 0;
};

namespace TeamEmploymentSystem
{
    TeamEmploymentContract contractFor(const Player& player);
    bool hasActiveContract(const Player& player);

    bool startExternalContract(
        Player& player,
        const std::string& contractId,
        const std::string& leaderName,
        const std::string& organizationId,
        const std::string& organizationName,
        const std::string& roleName,
        int salaryEconomyUnits,
        int payIntervalDays = 7,
        int durationDays = -1
    );

    int dueSalaryPeriods(const Player& player);
    long long dueSalaryEconomyUnits(const Player& player);
    TeamSalarySettlement collectDueSalary(Player& player);

    bool resign(Player& player, const std::string& reason = "depart_volontaire");
    bool dismiss(Player& player, const std::string& reason = "licenciement");
    bool dismissForDiscipline(
        Player& player,
        const std::string& reason = "faute_professionnelle",
        int salaryRetentionPercent = 75
    );
    bool finishContract(Player& player, const std::string& reason = "contrat_termine");
    bool finalizeIfExpired(Player& player);

    void recordEmployerMissionCompleted(Player& player, const std::string& questId, const std::string& title);
    void recordEmployerMissionFailed(Player& player, const std::string& questId, const std::string& title);
};
