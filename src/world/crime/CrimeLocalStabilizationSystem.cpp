#include "world/crime/CrimeLocalStabilizationSystem.hpp"

#include "entity/Player.hpp"
#include "quest/Quest.hpp"
#include "world/crime/VaultTheftNetworkCatalog.hpp"

#include <algorithm>

namespace
{
    std::string cityForContact(const PlayerCrimeContactState& state)
    {
        if (!state.homeCityId.empty()) return state.homeCityId;
        const VaultTheftNetworkContact* definition = VaultTheftNetworkCatalog::findById(state.contactId);
        return definition != nullptr ? definition->cityId : std::string{};
    }

    std::string questCityId(const Quest& quest)
    {
        const std::string suffix = " / guilde locale";
        const std::size_t suffixPos = quest.location.find(suffix);
        if (suffixPos != std::string::npos && suffixPos > 0)
            return quest.location.substr(0, suffixPos);

        const std::string prefix = "crime_local_";
        if (quest.id.rfind(prefix, 0) != 0) return {};

        const std::size_t dayPos = quest.id.rfind("_day");
        if (dayPos == std::string::npos || dayPos <= prefix.size()) return {};

        const std::string body = quest.id.substr(prefix.size(), dayPos - prefix.size());
        static const std::vector<std::string> kinds = {
            "merchant_check_", "route_watch_", "witness_followup_"
        };
        for (const std::string& kind : kinds)
        {
            if (body.rfind(kind, 0) == 0 && body.size() > kind.size())
                return body.substr(kind.size());
        }
        return {};
    }

    int strengthFor(const Quest& quest)
    {
        if (quest.id.find("witness_followup") != std::string::npos) return 3;
        if (quest.id.find("route_watch") != std::string::npos) return 2;
        return 1;
    }

    bool alreadyApplied(const Player& player, const std::string& questId)
    {
        for (const PlayerHistoricalEvent& event : player.getHistoricalEvents())
        {
            if (event.category == "crime_local_stabilization" && event.subjectId == questId)
                return true;
        }
        return false;
    }
}

CrimeLocalStabilizationResult CrimeLocalStabilizationSystem::applyQuestTurnIn(Player& player, const Quest& quest)
{
    CrimeLocalStabilizationResult result;
    if (quest.origin != "Conséquence locale" || quest.id.rfind("crime_local_", 0) != 0) return result;
    if (!quest.turnedIn || !quest.completed || alreadyApplied(player, quest.id)) return result;

    result.cityId = questCityId(quest);
    if (result.cityId.empty()) return result;

    result.strength = strengthFor(quest);
    const int pressureReduction = 3 + result.strength * 2;
    const int rivalryReduction = result.strength >= 3 ? 2 : 1;

    for (PlayerCrimeContactState& state : player.getCrimeContactStates())
    {
        if (cityForContact(state) != result.cityId || state.arrested) continue;

        const int beforePressure = state.pressure;
        const int beforeRivalry = state.rivalry;
        state.pressure = std::max(0, state.pressure - pressureReduction);
        state.rivalry = std::max(0, state.rivalry - rivalryReduction);
        state.lastInteractionDay = std::max(state.lastInteractionDay, player.getWorldDaysElapsed());

        const int pressureDelta = beforePressure - state.pressure;
        const int rivalryDelta = beforeRivalry - state.rivalry;
        if (pressureDelta > 0 || rivalryDelta > 0)
        {
            ++result.contactsCalmed;
            result.totalPressureReduced += pressureDelta;
            result.totalRivalryReduced += rivalryDelta;
        }
    }

    player.recordCanonicalEvent(
        "crime_local_stabilizations",
        result.cityId,
        "Suivi local terminé : " + quest.title,
        result.strength);
    player.recordHistoricalEvent(
        "crime_local_stabilization",
        quest.id,
        "Le suivi '" + quest.title + "' aide la ville à distinguer faits, routes perturbées et rumeurs. La pression baisse sans effacer les rivalités durables.",
        true);

    result.applied = true;
    result.lines.push_back("Stabilisation locale : le suivi de guilde réduit une partie de la pression née des derniers remous.");
    if (result.contactsCalmed > 0)
    {
        result.lines.push_back(std::to_string(result.contactsCalmed)
            + " contact(s) criminel(s) connu(s) deviennent légèrement moins sous pression ; les rivalités profondes ne disparaissent pas pour autant.");
    }
    result.lines.push_back("La récupération reste progressive : terminer un suivi aide, mais ne remet pas instantanément la ville dans son état précédent.");
    return result;
}
