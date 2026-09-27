#include "combat/ally/DuoMasterySystem.hpp"
#include "entity/Player.hpp"
#include <algorithm>
#include <cctype>

namespace
{
std::string low(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

bool has(const std::string& text, const std::string& token)
{
    return low(text).find(low(token)) != std::string::npos;
}
}

std::string DuoMasterySystem::pairKey(const std::string& firstName, const std::string& secondName)
{
    if (firstName <= secondName) return firstName + "|" + secondName;
    return secondName + "|" + firstName;
}

int DuoMasterySystem::experience(const Player& player, const std::string& pairKeyValue)
{
    int total = 0;
    const std::string prefix = pairKeyValue + ":";
    for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
    {
        if (record.category == "techniques_combinees_alliees" && record.key.rfind(prefix, 0) == 0)
        {
            total += std::max(0, record.count);
        }
    }
    return total;
}

int DuoMasterySystem::masteryTier(int experienceValue)
{
    if (experienceValue >= 12) return 3;
    if (experienceValue >= 7) return 2;
    if (experienceValue >= 3) return 1;
    return 0;
}

std::string DuoMasterySystem::masteryLabel(int tier)
{
    if (tier >= 3) return "signature commune";
    if (tier == 2) return "duo rodé";
    if (tier == 1) return "repères communs";
    return "coordination improvisée";
}

DuoCombatRole DuoMasterySystem::roleForJob(const std::string& job)
{
    if (has(job, "gardien") || has(job, "tank") || has(job, "protecteur")) return DuoCombatRole::Tank;
    if (has(job, "soigneur") || has(job, "mage d'appui") || has(job, "intendant") || has(job, "support")) return DuoCombatRole::Support;
    if (has(job, "archer") || has(job, "mage") || has(job, "tireur") || has(job, "arbalétrier")) return DuoCombatRole::Ranged;
    if (has(job, "assassin") || has(job, "roublard") || has(job, "brigand") || has(job, "lancier") || has(job, "duelliste")) return DuoCombatRole::Assault;
    return DuoCombatRole::Other;
}

DuoTechniquePlan DuoMasterySystem::techniqueForJobs(const std::string& firstJob, const std::string& secondJob)
{
    const DuoCombatRole first = roleForJob(firstJob);
    const DuoCombatRole second = roleForJob(secondJob);
    DuoTechniquePlan plan;

    if (first == DuoCombatRole::Tank && second == DuoCombatRole::Tank)
    {
        plan.name = "Mur en mouvement";
        plan.powerPercent = 84;
        plan.weakening = 18;
        plan.playerGuardPercent = 14;
    }
    else if (first == DuoCombatRole::Support && second == DuoCombatRole::Support)
    {
        plan.name = "Relais vital";
        plan.powerPercent = 64;
        plan.vulnerabilityPercent = 8;
        plan.playerHealPercent = 9;
    }
    else if ((first == DuoCombatRole::Tank && second == DuoCombatRole::Assault)
        || (second == DuoCombatRole::Tank && first == DuoCombatRole::Assault))
    {
        plan.name = "Brèche sous garde";
        plan.powerPercent = 112;
        plan.vulnerabilityPercent = 18;
        plan.weakening = 8;
    }
    else if ((first == DuoCombatRole::Support && second != DuoCombatRole::Support)
        || (second == DuoCombatRole::Support && first != DuoCombatRole::Support))
    {
        plan.name = "Faille relayée";
        plan.powerPercent = 92;
        plan.vulnerabilityPercent = 24;
    }
    else if (first == DuoCombatRole::Ranged && second == DuoCombatRole::Ranged)
    {
        plan.name = "Feu croisé";
        plan.powerPercent = 120;
        plan.vulnerabilityPercent = 10;
    }
    else if ((first == DuoCombatRole::Ranged) != (second == DuoCombatRole::Ranged))
    {
        plan.name = "Croisement de lignes";
        plan.powerPercent = 108;
        plan.weakening = 10;
    }
    return plan;
}

int DuoMasterySystem::coordinationChance(int maturityTotal, int rankTotal, int experienceValue, bool bondsOath)
{
    int chance = 62
        + std::max(0, maturityTotal) * 4
        + std::max(0, rankTotal) * 2
        + std::min(15, std::max(0, experienceValue) * 3);
    if (bondsOath) chance += 6;
    return std::clamp(chance, 62, 95);
}
