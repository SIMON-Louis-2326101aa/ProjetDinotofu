#include "entity/Player.hpp"
#include "world/LocalReputationRepairSystem.hpp"
#include "world/LocalReputationSystem.hpp"
#include <cassert>

int main()
{
    Player player;
    const std::string city = player.getCurrentCityId();
    player.recordCanonicalEvent("reputation_locale_negative", "incident_test", "Incident local de test", 20);
    player.getInventory().earnEconomyUnits(500);

    const int initial = LocalReputationSystem::score(player, city);
    assert(initial == -20);
    assert(LocalReputationRepairSystem::fineCostEconomyUnits(initial) > 0);
    assert(LocalReputationRepairSystem::fineReputationGain(initial) > 0);
    assert(LocalReputationRepairSystem::canPayFine(player, city));

    const int goldBefore = player.getInventory().getEconomyUnits();
    const auto fineLines = LocalReputationRepairSystem::payFine(player, city);
    assert(!fineLines.empty());
    assert(player.getInventory().getEconomyUnits() < goldBefore);
    const int afterFine = LocalReputationSystem::score(player, city);
    assert(afterFine > initial && afterFine < 0);

    assert(LocalReputationRepairSystem::canPerformCommunityService(player, city));
    const int timeBefore = player.getWorldDayProgressUnits();
    const auto serviceLines = LocalReputationRepairSystem::performCommunityService(player, city);
    assert(!serviceLines.empty());
    assert(player.getWorldDayProgressUnits() != timeBefore || player.getWorldDaysElapsed() > 0);
    const int afterService = LocalReputationSystem::score(player, city);
    assert(afterService == afterFine + 3);
    assert(!LocalReputationRepairSystem::canPerformCommunityService(player, city));
    return 0;
}
