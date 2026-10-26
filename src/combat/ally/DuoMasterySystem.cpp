#include "combat/ally/DuoMasterySystem.hpp"
#include "combat/ally/RecruitedAllyCombatRoster.hpp"
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

int exactCount(const Player& player, const std::string& category, const std::string& key)
{
    for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
    {
        if (record.category == category && record.key == key) return std::max(0, record.count);
    }
    return 0;
}

int prefixTotal(const Player& player, const std::string& category, const std::string& prefix)
{
    int total = 0;
    for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
    {
        if (record.category == category && record.key.rfind(prefix, 0) == 0)
        {
            total += std::max(0, record.count);
        }
    }
    return total;
}


}

std::string DuoMasterySystem::pairKey(const std::string& firstName, const std::string& secondName)
{
    if (firstName <= secondName) return firstName + "|" + secondName;
    return secondName + "|" + firstName;
}

int DuoMasterySystem::experience(const Player& player, const std::string& pairKeyValue)
{
    const std::string prefix = pairKeyValue + ":";
    const int successes = prefixTotal(player, "techniques_combinees_alliees", prefix);
    const int partials = prefixTotal(player, "techniques_combinees_partielles", prefix);
    const int sharedDays = prefixTotal(player, "duo_combats_partages", pairKeyValue + ":jour_");
    const int orderDays = prefixTotal(player, "duo_ordres_coordination", pairKeyValue + ":jour_");

    // Une technique réussie reste la meilleure preuve, mais les vrais jours de combat et les
    // ordres vécus ensemble comptent aussi. Les traces quotidiennes sont uniques : farmer le
    // même petit combat en boucle dans la même journée ne gonfle pas la maîtrise.
    return successes
        + std::min(6, partials)
        + std::min(8, sharedDays)
        + std::min(5, orderDays);
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

DuoBondProfile DuoMasterySystem::bondProfile(const Player& player, const std::string& firstName, const std::string& secondName)
{
    DuoBondProfile profile;
    const std::string key = pairKey(firstName, secondName);
    const std::string prefix = key + ":";
    profile.successfulCombos = prefixTotal(player, "techniques_combinees_alliees", prefix);
    profile.partialCombos = prefixTotal(player, "techniques_combinees_partielles", prefix);
    profile.failedCombos = exactCount(player, "techniques_combinees_ratees", key);
    profile.sharedCombatDays = prefixTotal(player, "duo_combats_partages", key + ":jour_");
    profile.coordinatedOrderDays = prefixTotal(player, "duo_ordres_coordination", key + ":jour_");
    profile.relationshipScore = (RecruitedAllyCombatRoster::relationshipScore(player, firstName) + RecruitedAllyCombatRoster::relationshipScore(player, secondName)) / 2;
    profile.bondScore = std::clamp(
        profile.successfulCombos * 6
        + profile.partialCombos * 3
        + std::min(4, profile.failedCombos) * 2
        + profile.sharedCombatDays * 5
        + profile.coordinatedOrderDays * 3
        + profile.relationshipScore / 4,
        0,
        100
    );
    if (profile.bondScore >= 82) profile.bondTier = 4;
    else if (profile.bondScore >= 60) profile.bondTier = 3;
    else if (profile.bondScore >= 35) profile.bondTier = 2;
    else if (profile.bondScore >= 15) profile.bondTier = 1;
    return profile;
}

std::string DuoMasterySystem::bondLabel(int tier)
{
    if (tier >= 4) return "lien exceptionnel";
    if (tier == 3) return "confiance profonde";
    if (tier == 2) return "confiance établie";
    if (tier == 1) return "habitudes communes";
    return "lien encore fragile";
}

bool DuoMasterySystem::recordSharedCombatDay(Player& player, const std::string& firstName, const std::string& secondName, const std::string& contextId)
{
    const std::string key = pairKey(firstName, secondName) + ":jour_" + std::to_string(player.getWorldDaysElapsed());
    if (exactCount(player, "duo_combats_partages", key) > 0) return false;
    player.recordCanonicalEvent("duo_combats_partages", key, firstName + " et " + secondName + " partagent un combat réel | contexte=" + contextId, 1);
    return true;
}

bool DuoMasterySystem::recordCoordinatedOrderDay(Player& player, const std::string& firstName, const std::string& secondName, const std::string& contextId)
{
    const std::string key = pairKey(firstName, secondName) + ":jour_" + std::to_string(player.getWorldDaysElapsed());
    if (exactCount(player, "duo_ordres_coordination", key) > 0) return false;
    player.recordCanonicalEvent("duo_ordres_coordination", key, firstName + " et " + secondName + " répondent ensemble à une consigne réelle | contexte=" + contextId, 1);
    return true;
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

int DuoMasterySystem::coordinationChance(int maturityTotal, int rankTotal, int experienceValue, int relationshipScore, bool bondsOath)
{
    int chance = 42
        + std::max(0, maturityTotal) * 4
        + std::max(0, rankTotal) * 2
        + std::min(18, std::max(0, experienceValue) * 2)
        + std::min(12, std::max(0, relationshipScore) / 8);
    if (bondsOath) chance += 6;
    return std::clamp(chance, 38, 95);
}

int DuoMasterySystem::combinedTechniqueRecoveryAfterAttempt()
{
    return 2;
}

int DuoMasterySystem::advanceCombinedTechniqueRecovery(int remainingEnemyResponses)
{
    return std::max(0, remainingEnemyResponses - 1);
}

bool DuoMasterySystem::combinedTechniqueReady(int remainingEnemyResponses)
{
    return remainingEnemyResponses <= 0;
}
