#include "combat/rival/RivalEmergenceSystem.hpp"

#include <algorithm>
#include <cctype>
#include <initializer_list>

namespace
{
    std::string lowerCopy(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
        return value;
    }

    bool containsAny(const std::string& value, const std::initializer_list<const char*>& needles)
    {
        const std::string lowered = lowerCopy(value);
        for (const char* needle : needles)
        {
            if (lowered.find(needle) != std::string::npos) return true;
        }
        return false;
    }
}

int RivalEmergenceSystem::calculateChancePercent(const RivalEmergenceContext& context)
{
    if (!context.eligible)
    {
        return 0;
    }

    // EN: An escape is ordinary by default. Even every favorable trace combined
    // leaves a real chance that the enemy simply disappears from the story.
    // FR: Une fuite est ordinaire par défaut. Même avec toutes les traces favorables,
    // l'ennemi garde une vraie chance de simplement disparaître de l'histoire.
    int chance = 6;
    if (context.elite) chance += 12;
    if (context.evolved) chance += 10;
    if (context.witnessed) chance += 8;
    if (context.memoryKept) chance += 6;
    if (context.rivalOath) chance += 22;
    chance += std::min(10, std::max(0, context.priorTraces) * 3);
    if (context.remainingHpPercent <= 8) chance += 4;

    return std::clamp(chance, 0, 65);
}

RivalEmergenceDecision RivalEmergenceSystem::evaluate(const RivalEmergenceContext& context, int rollPercent)
{
    RivalEmergenceDecision decision;
    decision.chancePercent = calculateChancePercent(context);
    decision.temperament = temperamentFor(context);
    decision.becomesRival = decision.chancePercent > 0
        && std::clamp(rollPercent, 1, 100) <= decision.chancePercent;

    if (!context.eligible)
    {
        decision.reason = "La fuite laisse une survie, pas une identité capable de porter une rivalité.";
    }
    else if (decision.becomesRival)
    {
        decision.reason = "La survie, les traces et la volonté propre de l'ennemi suffisent à fixer une identité.";
    }
    else
    {
        decision.reason = "Fuite ordinaire : aucun lien personnel durable ne se forme avec cet ennemi.";
    }

    return decision;
}

int RivalEmergenceSystem::minimumReturnDelayDays(int previousReturns)
{
    return 2 + std::min(4, std::max(0, previousReturns));
}

int RivalEmergenceSystem::returnChancePercent(
    bool localTrace,
    bool rivalOath,
    bool memoryKept,
    int previousReturns,
    int daysSinceLastSeen
)
{
    if (daysSinceLastSeen < minimumReturnDelayDays(previousReturns))
    {
        return 0;
    }

    int chance = localTrace ? 18 : 4;
    if (rivalOath) chance += 7;
    if (memoryKept) chance += 4;
    chance -= std::min(8, std::max(0, previousReturns) * 2);
    return std::clamp(chance, 0, 28);
}

std::string RivalEmergenceSystem::temperamentFor(const RivalEmergenceContext& context)
{
    if (context.elite || context.evolved)
    {
        return "endurci";
    }
    if (containsAny(context.behaviorArchetype, {"voleur", "sournois", "opportuniste", "duelliste"}))
    {
        return "calculateur";
    }
    if (containsAny(context.behaviorArchetype, {"meute", "prédateur", "predateur", "chasseur"}))
    {
        return "traqueur";
    }
    if (containsAny(context.behaviorArchetype, {"gobelin", "kobold", "rameuteur"}))
    {
        return "rancunier";
    }
    return "survivant prudent";
}
