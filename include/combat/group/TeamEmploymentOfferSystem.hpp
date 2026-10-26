#pragma once

#include <string>

class Player;

struct TeamEmploymentOffer
{
    bool available = false;
    std::string contractId;
    std::string leaderName;
    std::string organizationId;
    std::string organizationName;
    std::string roleName;
    std::string description;
    int salaryEconomyUnits = 0;
    int payIntervalDays = 7;
    int durationDays = 14;
};

namespace TeamEmploymentOfferSystem
{
    TeamEmploymentOffer localOfferFor(const Player& player);
    bool acceptLocalOffer(Player& player);
}
