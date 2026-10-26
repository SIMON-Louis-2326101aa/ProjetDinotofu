#include <cassert>

#include "class_system/ClassCatalog.hpp"
#include "combat/death/PlayerDeathResolver.hpp"
#include "entity/Player.hpp"
#include "story/HestiaArcSystem.hpp"

int main()
{
    Player player("Sacrifice", ClassCatalog::createClassByName("Guerrier"));
    assert(HestiaArcSystem::completeMainStoryRescue(player, "active_party", false));
    assert(HestiaArcSystem::setAwakeningStage(player, 1));
    player.takeDamage(player.getHp() + player.getMaxHp() + 100);
    assert(player.isDead());

    const auto resolution = PlayerDeathResolver::resolveImmediate(player, "unit_test");
    assert(resolution.handled);
    assert(resolution.hestiaSacrifice);
    assert(!player.isDead());
    assert(player.getHp() > 0);
    assert(!player.getHestiaStoryState().materialized);
    assert(player.getHestiaStoryState().sacrificeConsumed);

    const auto second = PlayerDeathResolver::resolveImmediate(player, "unit_test_again");
    assert(!second.handled);

    Player remote("Remote", ClassCatalog::createClassByName("Guerrier"));
    assert(HestiaArcSystem::completeMainStoryRescue(remote, "guild_diane", false));
    assert(HestiaArcSystem::setAwakeningStage(remote, 1));
    remote.takeDamage(remote.getHp() + remote.getMaxHp() + 100);
    assert(remote.isDead());
    const auto remoteResolution = PlayerDeathResolver::resolveImmediate(remote, "remote_test");
    assert(!remoteResolution.handled);
    assert(remote.isDead());
    assert(remote.getHestiaStoryState().materialized);

    return 0;
}
