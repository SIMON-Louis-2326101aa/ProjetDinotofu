#pragma once

#include <string>

class Random;

enum class EmployerTacticalOrderKind
{
    None,
    ProtectSupport,
    FocusFire,
    HoldLine,
    ExtractWounded,
    ControlledBreakthrough
};

struct EmployerTacticalOrderContext
{
    int activeEnemyCount = 1;
    int lowHpAllies = 0;
    int criticalHpAllies = 0;
    bool supportMemberActive = false;
    bool reserveAvailable = false;
    bool bossContext = false;
    int leaderIntelligence = 60;
    int professionalTrust = 70;
};

struct EmployerTacticalOrder
{
    EmployerTacticalOrderKind kind = EmployerTacticalOrderKind::None;
    std::string code;
    std::string title;
    std::string targetName;
    std::string targetId;
    std::string instruction;
    std::string rationale;
};

namespace EmployerTacticalOrderSystem
{
    EmployerTacticalOrder decide(const EmployerTacticalOrderContext& context, Random& random);
    std::string label(EmployerTacticalOrderKind kind);
}
