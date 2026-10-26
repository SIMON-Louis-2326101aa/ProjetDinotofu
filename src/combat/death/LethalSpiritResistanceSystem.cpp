#include "combat/death/LethalSpiritResistanceSystem.hpp"

#include "core/Random.hpp"
#include "entity/Player.hpp"

#include <sstream>
#include <string>

namespace
{
    constexpr const char* SpiritCategory = "resistance_spirituelle_lethale";

    std::string buildLabel(const LethalSpiritResistanceResult& result, const char* contextId)
    {
        std::ostringstream out;
        out << "contexte=" << (contextId != nullptr ? contextId : "inconnu")
            << " | succes=" << result.successes
            << " | echecs=" << result.failures
            << " | issue=" << (result.survived ? "refus_de_lacher_prise" : "esprit_epuise")
            << " | jets=";
        for (std::size_t i = 0; i < result.rolls.size(); ++i)
        {
            if (i != 0) out << ',';
            out << result.rolls[i];
        }
        return out.str();
    }
}

bool LethalSpiritResistanceSystem::hasAlreadyConsumedFirstResistance(const Player& player)
{
    const int consumed = player.getCanonicalJournalCategoryTotal(SpiritCategory);
    const int rearmed = player.getCanonicalJournalCategoryTotal("cheat_spirit_rearm");
    return consumed > rearmed;
}

bool LethalSpiritResistanceSystem::canAttempt(const Player& player)
{
    return player.isDead() && !hasAlreadyConsumedFirstResistance(player);
}

LethalSpiritResistanceResult LethalSpiritResistanceSystem::tryFirstLethalResistance(
    Player& player,
    Random& random,
    const char* contextId)
{
    LethalSpiritResistanceResult result;
    result.available = canAttempt(player);
    if (!result.available) return result;

    result.consumed = true;

    while (result.successes < RequiredSuccesses && result.failures < RequiredFailures)
    {
        const int roll = random.rollD20();
        result.rolls.push_back(roll);
        if (roll >= D20SuccessThreshold)
        {
            ++result.successes;
        }
        else
        {
            ++result.failures;
        }
    }

    result.survived = result.successes >= RequiredSuccesses;
    if (result.survived)
    {
        player.reviveWithHealthPercentage(1);
        if (player.getHp() <= 0) player.heal(1);
    }

    player.recordCanonicalEvent(
        SpiritCategory,
        "premiere_et_unique_resistance",
        buildLabel(result, contextId),
        1
    );

    if (result.survived)
    {
        player.recordCanonicalEvent(
            "survies_lethales_esprit",
            std::string(contextId != nullptr ? contextId : "inconnu") + ":jour_" + std::to_string(player.getWorldDaysElapsed()),
            "Le personnage a refusé de lâcher prise lors d'une résistance spirituelle létale.",
            1
        );
        player.grantTitle("Pas encore.");
        if (result.successes >= RequiredSuccesses) player.grantTitle("Trois fois non");
    }

    return result;
}
