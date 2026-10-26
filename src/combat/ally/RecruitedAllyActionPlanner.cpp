#include "combat/ally/RecruitedAllyActionPlanner.hpp"

#include "core/Random.hpp"

#include <algorithm>

bool RecruitedAllyActionPlanner::isSupportProfile(const RecruitedAllyCombatSupport& ally)
{
    return RecruitedAllyCombatRoster::jobContains(ally, "soigneur")
        || RecruitedAllyCombatRoster::jobContains(ally, "mage d'appui")
        || RecruitedAllyCombatRoster::jobContains(ally, "support")
        || RecruitedAllyCombatRoster::jobContains(ally, "prêtre")
        || RecruitedAllyCombatRoster::jobContains(ally, "clerc")
        || RecruitedAllyCombatRoster::jobContains(ally, "médecin")
        || RecruitedAllyCombatRoster::jobContains(ally, "infirmier")
        || RecruitedAllyCombatRoster::jobContains(ally, "druide")
        || RecruitedAllyCombatRoster::jobContains(ally, "chaman")
        || RecruitedAllyCombatRoster::jobContains(ally, "shaman");
}

bool RecruitedAllyActionPlanner::isGuardProfile(const RecruitedAllyCombatSupport& ally)
{
    return RecruitedAllyCombatRoster::jobContains(ally, "gardien")
        || RecruitedAllyCombatRoster::jobContains(ally, "tank")
        || RecruitedAllyCombatRoster::jobContains(ally, "protecteur")
        || RecruitedAllyCombatRoster::jobContains(ally, "paladin");
}

int RecruitedAllyActionPlanner::naturalSupportChance(const RecruitedAllyCombatSupport& ally)
{
    int chance = 12 + ally.currentRankIndex * 4 + ally.supportKitQuality * 2
        + ally.estimatedLevel / 22 + RecruitedAllyCombatRoster::maturityTier(ally) * 3;
    if (isSupportProfile(ally)) chance += 12;
    if (RecruitedAllyCombatRoster::maturityTier(ally) <= 1) chance = chance * 65 / 100;
    return std::clamp(chance, 5, 60);
}

int RecruitedAllyActionPlanner::naturalTechniqueChance(
    const RecruitedAllyCombatSupport& ally,
    int enemyCount,
    bool bossContext)
{
    int chance = 7 + ally.currentRankIndex * 4 + ally.estimatedLevel / 20
        + RecruitedAllyCombatRoster::maturityTier(ally) * 4;
    if (RecruitedAllyCombatRoster::jobContains(ally, "roublard")
        || RecruitedAllyCombatRoster::jobContains(ally, "assassin")
        || RecruitedAllyCombatRoster::jobContains(ally, "brigand"))
    {
        chance += 11;
    }
    else if ((RecruitedAllyCombatRoster::jobContains(ally, "archer")
        || RecruitedAllyCombatRoster::jobContains(ally, "mage")
        || RecruitedAllyCombatRoster::jobContains(ally, "lancier")) && enemyCount >= 2)
    {
        chance += 9;
    }
    else if (isSupportProfile(ally))
    {
        chance += 6;
    }

    if (bossContext) chance += 4;
    if (RecruitedAllyCombatRoster::maturityTier(ally) <= 1) chance = chance * 55 / 100;
    if (RecruitedAllyCombatRoster::maturityTier(ally) >= 4) chance += 5;
    return std::clamp(chance, 3, bossContext ? 62 : 56);
}

int RecruitedAllyActionPlanner::orderedTechniqueChance(const RecruitedAllyCombatSupport& ally, bool forced)
{
    const int base = forced ? 56 : 28;
    const int rankBonus = ally.currentRankIndex * (forced ? 7 : 8);
    const int levelBonus = ally.estimatedLevel / (forced ? 10 : 12);
    return std::clamp(base + rankBonus + levelBonus, forced ? 58 : 25, forced ? 96 : 86);
}

RecruitedAllyActionDecision RecruitedAllyActionPlanner::decide(
    const RecruitedAllyCombatSupport& ally,
    const RecruitedAllyActionContext& context,
    Random& random)
{
    if (ally.currentHp <= 0)
        return {RecruitedAllyActionType::Hold, false, "allié hors combat"};
    if (!context.hasEnemy)
        return {RecruitedAllyActionType::Hold, false, "aucune cible active"};

    if (ally.currentHp * 3 < std::max(1, ally.maxHp) && ally.healingPotionCharges > 0)
    {
        return {RecruitedAllyActionType::SelfHeal, false, "survie personnelle prioritaire"};
    }

    if (context.orderedHeal)
    {
        if (context.leaderPresent && context.leaderCanBeHealed && context.leaderHpPercent < 100)
            return {RecruitedAllyActionType::HealLeader, true, "ordre de soin du chef"};
        if (context.woundedAllyAvailable && context.woundedAllyHpPercent < 100)
            return {RecruitedAllyActionType::HealAlly, true, "ordre de soin redirigé vers un allié blessé"};
    }

    if (isSupportProfile(ally))
    {
        if (context.leaderPresent && context.leaderCanBeHealed && context.leaderHpPercent < 70
            && random.between(1, 100) <= naturalSupportChance(ally))
        {
            return {RecruitedAllyActionType::HealLeader, false, "soutien naturel du chef"};
        }
        if ((!context.leaderPresent || context.leaderHpPercent <= 0)
            && context.woundedAllyAvailable && context.woundedAllyHpPercent < 58
            && random.between(1, 100) <= naturalSupportChance(ally))
        {
            return {RecruitedAllyActionType::HealAlly, false, "soutien naturel d'un allié blessé"};
        }
    }

    if (context.orderedGuard)
    {
        return {RecruitedAllyActionType::Guard, true, "ordre de garde"};
    }

    if (context.techniqueAllowed && context.techniqueAvailable)
    {
        int chance = naturalTechniqueChance(ally, std::max(1, context.enemyCount), context.bossContext);
        if (context.orderedTechnique)
            chance = orderedTechniqueChance(ally, true);
        else if (context.hasDirectOrder)
            chance = orderedTechniqueChance(ally, false);

        if (random.between(1, 100) <= chance)
        {
            return {RecruitedAllyActionType::Technique, context.orderedTechnique || context.hasDirectOrder,
                context.orderedTechnique ? "technique demandée par le chef" : "fenêtre tactique détectée"};
        }
    }

    return {RecruitedAllyActionType::Attack, false, "attaque standard"};
}

std::string RecruitedAllyActionPlanner::actionTypeLabel(RecruitedAllyActionType type)
{
    switch (type)
    {
        case RecruitedAllyActionType::SelfHeal: return "soin personnel";
        case RecruitedAllyActionType::HealLeader: return "soin du chef";
        case RecruitedAllyActionType::HealAlly: return "soin d'allié";
        case RecruitedAllyActionType::Guard: return "garde";
        case RecruitedAllyActionType::Technique: return "technique";
        case RecruitedAllyActionType::Attack: return "attaque";
        case RecruitedAllyActionType::Hold:
        default: return "attente";
    }
}
