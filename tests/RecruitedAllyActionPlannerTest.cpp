#include "combat/ally/RecruitedAllyActionPlanner.hpp"
#include "core/Random.hpp"

#include <cassert>

namespace
{
    RecruitedAllyCombatSupport makeAlly(const std::string& job)
    {
        RecruitedAllyCombatSupport ally;
        ally.name = "Aline";
        ally.job = job;
        ally.currentHp = 100;
        ally.maxHp = 100;
        ally.estimatedLevel = 40;
        ally.currentRankIndex = 3;
        ally.supportKitQuality = 4;
        ally.weaponQuality = 4;
        ally.healingPotionCharges = 1;
        return ally;
    }
}

int main()
{
    {
        auto ally = makeAlly("Soigneur");
        ally.currentHp = 20;
        RecruitedAllyActionContext context;
        Random random(1u);
        const auto decision = RecruitedAllyActionPlanner::decide(ally, context, random);
        assert(decision.type == RecruitedAllyActionType::SelfHeal);
    }

    {
        auto ally = makeAlly("Soigneur");
        RecruitedAllyActionContext context;
        context.orderedHeal = true;
        context.leaderPresent = true;
        context.leaderHpPercent = 40;
        Random random(2u);
        const auto decision = RecruitedAllyActionPlanner::decide(ally, context, random);
        assert(decision.type == RecruitedAllyActionType::HealLeader);
        assert(decision.followsOrder);
    }

    {
        auto ally = makeAlly("Soigneur");
        RecruitedAllyActionContext context;
        context.orderedHeal = true;
        context.leaderPresent = false;
        context.leaderCanBeHealed = false;
        context.woundedAllyAvailable = true;
        context.woundedAllyHpPercent = 25;
        Random random(3u);
        const auto decision = RecruitedAllyActionPlanner::decide(ally, context, random);
        assert(decision.type == RecruitedAllyActionType::HealAlly);
    }

    {
        auto ally = makeAlly("Gardien");
        RecruitedAllyActionContext context;
        context.orderedGuard = true;
        Random random(4u);
        const auto decision = RecruitedAllyActionPlanner::decide(ally, context, random);
        assert(decision.type == RecruitedAllyActionType::Guard);
        assert(decision.followsOrder);
    }

    {
        auto ally = makeAlly("Assassin");
        ally.currentRankIndex = 6;
        ally.estimatedLevel = 100;
        RecruitedAllyActionContext context;
        context.orderedTechnique = true;
        context.hasDirectOrder = true;
        context.techniqueAvailable = true;
        Random random(5u);
        const auto decision = RecruitedAllyActionPlanner::decide(ally, context, random);
        assert(decision.type == RecruitedAllyActionType::Technique);
    }

    {
        auto ally = makeAlly("Guerrier");
        RecruitedAllyActionContext context;
        context.techniqueAvailable = false;
        Random random(6u);
        const auto decision = RecruitedAllyActionPlanner::decide(ally, context, random);
        assert(decision.type == RecruitedAllyActionType::Attack);
    }

    assert(RecruitedAllyActionPlanner::isSupportProfile(makeAlly("Médecin de terrain")));
    assert(RecruitedAllyActionPlanner::isGuardProfile(makeAlly("Protecteur")));
    return 0;
}
