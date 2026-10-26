#include "combat/ally/RecruitedAllyInitiativeSystem.hpp"

#include <algorithm>
#include <cctype>
#include <numeric>
#include <string>

namespace
{
    std::string lower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool containsAny(const std::string& text, std::initializer_list<const char*> terms)
    {
        const std::string value = lower(text);
        for (const char* term : terms)
        {
            if (value.find(term) != std::string::npos) return true;
        }
        return false;
    }
}

int RecruitedAllyInitiativeSystem::baseInitiativeScore(const RecruitedAllyCombatSupport& ally)
{
    int score = 40;
    score += std::clamp(ally.estimatedLevel / 4, 0, 24);
    score += std::clamp(ally.currentRankIndex * 3, 0, 18);
    score += std::clamp(ally.weaponQuality, 0, 5);

    const std::string profile = ally.job + " " + ally.trait + " " + ally.motivation + " " + ally.race;
    if (containsAny(profile, {"éclaireur", "eclaireur", "assassin", "voleur", "roublard", "archer", "rôdeur", "rodeur", "rapide", "agile", "vif"})) score += 10;
    if (containsAny(profile, {"fée", "fee", "halfelin", "kitsune"})) score += 3;
    if (containsAny(profile, {"garde", "tank", "protecteur", "lourd", "massif", "lent", "brute"})) score -= 7;
    if (containsAny(profile, {"blessé", "blesse", "épuisé", "epuise"})) score -= 4;

    // Persistent condition only nudges ordering inside the normal ally window. It never creates
    // or removes a turn, and the operational leader rule below remains stronger than speed.
    score -= std::clamp(ally.fatiguePercent / 7, 0, 14);
    score += std::clamp((ally.moralePercent - 50) / 10, -4, 4);
    if (ally.readinessPercent <= 30) score -= 5;

    return std::clamp(score, 10, 95);
}

std::vector<int> RecruitedAllyInitiativeSystem::orderedActiveIndices(
    const std::vector<RecruitedAllyCombatSupport>& allies)
{
    std::vector<int> indices(allies.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::stable_sort(indices.begin(), indices.end(), [&](int left, int right) {
        const auto& a = allies[static_cast<std::size_t>(left)];
        const auto& b = allies[static_cast<std::size_t>(right)];
        // Operational leader priority is a separate combat rule and remains stronger than speed.
        if (a.teamLeader != b.teamLeader) return a.teamLeader > b.teamLeader;
        const int aScore = baseInitiativeScore(a);
        const int bScore = baseInitiativeScore(b);
        if (aScore != bScore) return aScore > bScore;
        return left < right;
    });
    return indices;
}
