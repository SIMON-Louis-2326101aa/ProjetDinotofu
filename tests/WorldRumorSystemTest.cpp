#include <cassert>

#include "class_system/ClassCatalog.hpp"
#include "entity/Player.hpp"
#include "world/rumor/WorldRumorSystem.hpp"

int main()
{
    Player player("Auditeur", ClassCatalog::createClassByName("Guerrier"));

    WorldRumorSystem::recordRumor(
        player,
        "red_bridge_masked_watchers",
        "masked_watchers",
        "Des voyageurs parlent de silhouettes masquées près du Pont Rouge.",
        WorldRumorSourceType::Tavern,
        35,
        0);
    WorldRumorStatus status = WorldRumorSystem::statusFor(player, "red_bridge_masked_watchers");
    assert(status.known);
    assert(!status.confirmed);
    assert(status.subjectId == "masked_watchers");
    assert(status.heardCount == 1);
    assert(status.credibility < 50);

    WorldRumorSystem::recordRumor(
        player,
        "red_bridge_masked_watchers",
        "masked_watchers",
        "Un garde confirme avoir trouvé des traces organisées près du pont.",
        WorldRumorSourceType::Guard,
        60,
        2);
    status = WorldRumorSystem::statusFor(player, "red_bridge_masked_watchers");
    assert(status.heardCount == 2);
    assert(status.evidenceLevel >= 2);
    assert(status.credibility > 50);

    WorldRumorSystem::confirmRumor(
        player,
        "red_bridge_masked_watchers",
        "masked_watchers",
        "Le joueur observe directement une unité traverser la zone sans piller les civils.",
        3);
    status = WorldRumorSystem::statusFor(player, "red_bridge_masked_watchers");
    assert(status.confirmed);
    assert(status.credibility >= 90);
    assert(status.evidenceLevel >= 3);

    WorldRumorSystem::contradictRumor(
        player,
        "red_bridge_masked_watchers",
        "masked_watchers",
        "Un détail populaire de la légende est démenti par un registre de route.");
    status = WorldRumorSystem::statusFor(player, "red_bridge_masked_watchers");
    assert(status.contradicted);

    const auto lines = WorldRumorSystem::buildSummaryLines(player, "red_bridge_masked_watchers");
    assert(lines.size() >= 4);
    return 0;
}
