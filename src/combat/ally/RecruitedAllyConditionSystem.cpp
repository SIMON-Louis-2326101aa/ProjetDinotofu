#include "combat/ally/RecruitedAllyConditionSystem.hpp"

#include "entity/Player.hpp"
#include "combat/ally/RecruitedAllyAspirationSystem.hpp"

#include <algorithm>
#include <cctype>

namespace
{
    int intField(const std::string& label, const std::string& fieldName, int fallback)
    {
        const std::string marker = fieldName + "=";
        const std::size_t pos = label.find(marker);
        if (pos == std::string::npos) return fallback;
        std::size_t start = pos + marker.size();
        std::size_t end = start;
        if (end < label.size() && label[end] == '-') ++end;
        while (end < label.size() && std::isdigit(static_cast<unsigned char>(label[end]))) ++end;
        if (end <= start) return fallback;
        try { return std::stoi(label.substr(start, end - start)); }
        catch (...) { return fallback; }
    }

    const PlayerJournalRecord* conditionRecord(const Player& player, const std::string& name)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == "etat_recrues_persistant" && record.key == name) return &record;
        }
        return nullptr;
    }

    int healthPercent(const RecruitedAllyCombatSupport& ally)
    {
        if (ally.maxHp <= 0) return 0;
        return std::clamp(ally.currentHp * 100 / ally.maxHp, 0, 100);
    }
}

RecruitedAllyConditionSnapshot RecruitedAllyConditionSystem::evaluate(
    const Player& player,
    const RecruitedAllyCombatSupport& ally)
{
    RecruitedAllyConditionSnapshot condition;
    const PlayerJournalRecord* record = conditionRecord(player, ally.name);
    int storedFatigue = 0;
    int storedInjuryStress = 0;
    int lastDay = player.getWorldDaysElapsed();
    if (record != nullptr)
    {
        storedFatigue = std::clamp(intField(record->label, "fatigue", 0), 0, 100);
        storedInjuryStress = std::clamp(intField(record->label, "trauma", 0), 0, 100);
        lastDay = std::max(0, record->lastDay);
    }

    condition.daysSinceLastStrain = std::max(0, player.getWorldDaysElapsed() - lastDay);
    // Two quiet days normally erase ordinary combat fatigue; heavy exhaustion takes longer.
    condition.fatiguePercent = std::clamp(storedFatigue - condition.daysSinceLastStrain * 18, 0, 100);
    // Injury stress recovers much more slowly than ordinary fatigue. It is a light persistent
    // memory of severe injuries/KO, not a psychiatric diagnosis or permanent debuff.
    condition.injuryStressPercent = std::clamp(storedInjuryStress - condition.daysSinceLastStrain * 5, 0, 100);

    const int relationship = ally.teamOrigin == AllyTeamOrigin::PersonalRecruit
        ? RecruitedAllyCombatRoster::relationshipScore(player, ally.name)
        : 50;
    const int hp = healthPercent(ally);
    int morale = 48 + relationship / 3;
    if (ally.teamOrigin == AllyTeamOrigin::PersonalRecruit)
    {
        morale += RecruitedAllyAspirationSystem::evaluate(player, ally).moraleModifier;
    }
    morale -= condition.fatiguePercent / 4;
    morale -= condition.injuryStressPercent / 5;
    if (hp <= 20) morale -= 18;
    else if (hp <= 45) morale -= 9;
    else if (hp >= 85) morale += 4;
    condition.moralePercent = std::clamp(morale, 5, 100);

    int readiness = 100 - condition.fatiguePercent / 2 - condition.injuryStressPercent / 4;
    if (hp <= 20) readiness -= 35;
    else if (hp <= 45) readiness -= 20;
    else if (hp <= 70) readiness -= 8;
    readiness += (condition.moralePercent - 50) / 5;
    condition.readinessPercent = std::clamp(readiness, 5, 100);
    condition.exhausted = condition.fatiguePercent >= 80 || condition.readinessPercent <= 30;
    condition.shaken = condition.injuryStressPercent >= 35;
    if (ally.currentHp <= 0 || condition.injuryStressPercent >= 70)
    {
        condition.injuryStateId = "severe_recovery";
        condition.injuryStateLabel = "séquelles sérieuses en récupération";
    }
    else if (condition.injuryStressPercent >= 40 || hp <= 20)
    {
        condition.injuryStateId = "significant_wound";
        condition.injuryStateLabel = "blessure importante en récupération";
    }
    else if (condition.injuryStressPercent >= 15 || hp <= 45)
    {
        condition.injuryStateId = "bruised";
        condition.injuryStateLabel = "contusions et fatigue de blessure";
    }
    return condition;
}

void RecruitedAllyConditionSystem::applyToCombatSupport(const Player& player, RecruitedAllyCombatSupport& ally)
{
    if (ally.teamOrigin != AllyTeamOrigin::PersonalRecruit || ally.temporaryNoReward) return;
    const RecruitedAllyConditionSnapshot condition = evaluate(player, ally);
    ally.fatiguePercent = condition.fatiguePercent;
    ally.moralePercent = condition.moralePercent;
    ally.readinessPercent = condition.readinessPercent;
}

void RecruitedAllyConditionSystem::recordCombatStrain(
    Player& player,
    const RecruitedAllyCombatSupport& ally,
    bool bossFight)
{
    if (ally.name.empty() || ally.teamOrigin != AllyTeamOrigin::PersonalRecruit || ally.temporaryNoReward || ally.turnsTaken <= 0) return;
    const RecruitedAllyConditionSnapshot before = evaluate(player, ally);
    const int hp = healthPercent(ally);
    int strain = 7 + std::min(15, ally.turnsTaken * 2) + (bossFight ? 8 : 0);
    if (hp <= 20) strain += 18;
    else if (hp <= 45) strain += 10;
    if (ally.currentHp <= 0) strain += 20;
    const int fatigue = std::clamp(before.fatiguePercent + strain, 0, 100);
    int traumaGain = 0;
    if (ally.currentHp <= 0) traumaGain += 28;
    else if (hp <= 15) traumaGain += 16;
    else if (hp <= 30) traumaGain += 8;
    if (bossFight && hp <= 45) traumaGain += 5;
    const int injuryStress = std::clamp(before.injuryStressPercent + traumaGain, 0, 100);
    const std::string label = ally.name
        + " | fatigue=" + std::to_string(fatigue)
        + " | trauma=" + std::to_string(injuryStress)
        + " | source=" + std::string(bossFight ? "boss" : "combat")
        + " | hp=" + std::to_string(hp)
        + " | tours=" + std::to_string(std::max(0, ally.turnsTaken));
    player.recordCanonicalEvent("etat_recrues_persistant", ally.name, label, 1);
}

bool RecruitedAllyConditionSystem::needsRecoveryCare(const Player& player, const RecruitedAllyCombatSupport& ally)
{
    const RecruitedAllyConditionSnapshot condition = evaluate(player, ally);
    return condition.injuryStressPercent >= 10 || condition.fatiguePercent >= 35;
}

RecruitedAllyRecoveryCareResult RecruitedAllyConditionSystem::provideRecoveryCare(
    Player& player,
    const RecruitedAllyCombatSupport& ally,
    int quality)
{
    RecruitedAllyRecoveryCareResult result;
    if (ally.name.empty() || ally.teamOrigin != AllyTeamOrigin::PersonalRecruit || ally.temporaryNoReward) return result;

    const RecruitedAllyConditionSnapshot before = evaluate(player, ally);
    result.fatigueBefore = before.fatiguePercent;
    result.injuryStressBefore = before.injuryStressPercent;
    result.injuryStateBefore = before.injuryStateLabel;
    if (before.fatiguePercent <= 0 && before.injuryStressPercent <= 0)
    {
        result.fatigueAfter = before.fatiguePercent;
        result.injuryStressAfter = before.injuryStressPercent;
        result.injuryStateAfter = before.injuryStateLabel;
        return result;
    }

    quality = std::clamp(quality, 1, 3);
    const int fatigueReduction = 6 + quality * 5;
    const int injuryReduction = 8 + quality * 7;
    const int fatigue = std::max(0, before.fatiguePercent - fatigueReduction);
    const int injuryStress = std::max(0, before.injuryStressPercent - injuryReduction);

    const std::string label = ally.name
        + " | fatigue=" + std::to_string(fatigue)
        + " | trauma=" + std::to_string(injuryStress)
        + " | source=care"
        + " | quality=" + std::to_string(quality);
    player.recordCanonicalEvent("etat_recrues_persistant", ally.name, label, 1);
    player.recordCanonicalEvent("soins_recuperation_recrues", ally.name, ally.name + " reçoit des soins de récupération adaptés", 1);

    const RecruitedAllyConditionSnapshot after = evaluate(player, ally);
    result.applied = true;
    result.fatigueAfter = after.fatiguePercent;
    result.injuryStressAfter = after.injuryStressPercent;
    result.injuryStateAfter = after.injuryStateLabel;
    return result;
}

std::string RecruitedAllyConditionSystem::summary(const RecruitedAllyConditionSnapshot& condition)
{
    std::string fatigue = condition.fatiguePercent >= 80 ? "épuisé"
        : condition.fatiguePercent >= 55 ? "fatigué"
        : condition.fatiguePercent >= 25 ? "un peu fatigué"
        : "repos correct";
    std::string morale = condition.moralePercent >= 75 ? "moral haut"
        : condition.moralePercent >= 50 ? "moral stable"
        : condition.moralePercent >= 30 ? "moral fragile"
        : "moral très bas";
    std::string trauma = condition.injuryStressPercent >= 65 ? " | très marqué"
        : condition.injuryStressPercent >= 35 ? " | marqué par les blessures"
        : condition.injuryStressPercent >= 15 ? " | encore secoué"
        : "";
    if (condition.injuryStateId != "none") trauma += " | " + condition.injuryStateLabel;
    return fatigue + " | " + morale + trauma + " | préparation " + std::to_string(condition.readinessPercent) + "%";
}
