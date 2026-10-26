#include <cassert>

#include "entity/Player.hpp"
#include "item/inventory/CarryLoadSystem.hpp"

int main()
{
    Player player;
    player.setAppearanceProfile(18, "Test", "Morphologie standard");
    assert(player.getBodyMorphology() == "Standard");
    const int standardCapacity = CarryLoadSystem::capacity(player);

    assert(player.canStartMorphologyProgram("Fine"));
    assert(player.canStartMorphologyProgram("Robuste"));
    assert(!player.canStartMorphologyProgram("Standard"));
    assert(player.startMorphologyProgram("Robuste"));
    assert(player.hasActiveMorphologyProgram());
    assert(player.getBodyMorphology() == "Standard");

    for (int i = 0; i < 8; ++i)
    {
        assert(player.performMorphologySession());
        assert(!player.performMorphologySession());
        if (i < 7) player.advanceWorldDays(1);
    }
    assert(!player.tryCompleteMorphologyProgram());
    player.advanceWorldDays(7); // day 14 from program start
    assert(player.tryCompleteMorphologyProgram());
    assert(player.getBodyMorphology() == "Robuste");
    assert(CarryLoadSystem::capacity(player) > standardCapacity);
    assert(player.hasTitle("Un corps façonné"));
    assert(player.hasTitle("Plus solide qu'avant"));
    assert(player.hasTitle("Taillé pour porter"));

    // No direct Robuste -> Fine jump: the program must go through Standard.
    assert(!player.canStartMorphologyProgram("Fine"));
    assert(player.startMorphologyProgram("Standard"));
    assert(player.cancelMorphologyProgram());
    assert(player.getBodyMorphology() == "Robuste");
    assert(!player.hasActiveMorphologyProgram());
    return 0;
}
