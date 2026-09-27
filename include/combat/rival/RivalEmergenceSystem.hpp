#ifndef INCLUDE_COMBAT_RIVAL_RIVALEMERGENCESYSTEM_HPP
#define INCLUDE_COMBAT_RIVAL_RIVALEMERGENCESYSTEM_HPP

#include <string>

// EN: Facts that can logically be known when an enemy escapes.
// FR: Faits qui peuvent être connus logiquement lorsqu'un ennemi s'enfuit.
struct RivalEmergenceContext
{
    bool eligible = true;
    bool elite = false;
    bool evolved = false;
    bool witnessed = false;
    bool memoryKept = false;
    bool rivalOath = false;
    int priorTraces = 0;
    int remainingHpPercent = 100;
    std::string behaviorArchetype;
};

struct RivalEmergenceDecision
{
    bool becomesRival = false;
    int chancePercent = 0;
    std::string temperament = "survivant prudent";
    std::string reason;
};

class RivalEmergenceSystem
{
public:
    static int calculateChancePercent(const RivalEmergenceContext& context);
    static RivalEmergenceDecision evaluate(const RivalEmergenceContext& context, int rollPercent);
    static int minimumReturnDelayDays(int previousReturns);
    static int returnChancePercent(
        bool localTrace,
        bool rivalOath,
        bool memoryKept,
        int previousReturns,
        int daysSinceLastSeen
    );

private:
    static std::string temperamentFor(const RivalEmergenceContext& context);
};

#endif
