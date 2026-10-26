#include <cassert>

#include "entity/Player.hpp"
#include "item/inventory/CarryLoadSystem.hpp"
#include "item/material/Material.hpp"

namespace
{
    Material bulk(int quantity)
    {
        return Material(
            "storage_title_test_bulk",
            "Lot de rangement",
            "Pile neutre servant uniquement à vérifier les titres de logistique.",
            "Composant",
            1,
            quantity
        );
    }
}

int main()
{
    // A real storage session must compare before/after, not grant from a single snapshot.
    Player storage;
    storage.getInventory().clearAll();
    storage.setAppearanceProfile(18, "Test", "Morphologie standard");
    const int capacity = CarryLoadSystem::capacity(storage);
    storage.getInventory().addMaterial(bulk(capacity * 17)); // ~170 %: deliberately very overloaded.
    const CarryLoadSummary before = CarryLoadSystem::evaluate(storage);
    assert(before.percent > 150 && before.percent < 200);
    assert(!storage.hasTitle("Grand ménage"));
    assert(!storage.hasTitle("Ça respire enfin"));

    storage.getInventory().clearAll();
    storage.evaluateStorageSessionTitles(before.used, before.capacity);
    assert(storage.hasTitle("Grand ménage"));
    assert(storage.hasTitle("Ça respire enfin"));

    // Returning almost immobilised after a genuine expedition can unlock the two dormant
    // exploration/logistics titles only when the run stayed below the 200 % hard block.
    Player expedition;
    expedition.getInventory().clearAll();
    expedition.setAppearanceProfile(18, "Test", "Morphologie standard");
    const int expeditionCapacity = CarryLoadSystem::capacity(expedition);
    const int targetLoad = (expeditionCapacity * 185 + 99) / 100;
    expedition.getInventory().addMaterial(bulk(targetLoad * 10));
    const CarryLoadSummary returnLoad = CarryLoadSystem::evaluate(expedition);
    assert(returnLoad.percent >= 180 && returnLoad.percent < 200);
    assert(expedition.getLootPickupPolicy() == "all");

    expedition.evaluateExplorationReturnTitles(190, 4);
    assert(expedition.hasTitle("Je garde tout"));
    assert(expedition.hasTitle("Déménageur"));

    Player blocked;
    blocked.getInventory().clearAll();
    blocked.setAppearanceProfile(18, "Test", "Morphologie standard");
    const int blockedCapacity = CarryLoadSystem::capacity(blocked);
    const int blockedTargetLoad = (blockedCapacity * 185 + 99) / 100;
    blocked.getInventory().addMaterial(bulk(blockedTargetLoad * 10));
    blocked.evaluateExplorationReturnTitles(200, 4);
    assert(!blocked.hasTitle("Je garde tout"));
    assert(!blocked.hasTitle("Déménageur"));

    return 0;
}
