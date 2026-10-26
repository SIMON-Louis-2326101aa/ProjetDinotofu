#pragma once

#include <string>

class Player;
struct RecruitedAllyCombatSupport;

struct RecruitedAllyAspirationSnapshot
{
    std::string aspirationId = "survive_together";
    std::string aspirationLabel = "revenir vivant avec le groupe";
    int satisfactionPercent = 50;
    int moraleModifier = 0;
    std::string summary;
};

namespace RecruitedAllyAspirationSystem
{
    // Lightweight aspiration derived from the recruit's authored motivation/job/trait and lived events.
    // It never invents a hidden backstory or permanent psychological state.
    RecruitedAllyAspirationSnapshot evaluate(const Player& player, const RecruitedAllyCombatSupport& ally);
}
