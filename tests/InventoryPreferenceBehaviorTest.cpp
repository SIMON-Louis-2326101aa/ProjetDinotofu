#include "class_system/ClassCatalog.hpp"
#include "entity/Player.hpp"
#include "interface/menu/inventory/InventoryDisplay.hpp"
#include "item/Inventory.hpp"
#include "item/material/MaterialCatalog.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    std::size_t findLineContaining(const std::vector<std::string>& lines, const std::string& needle)
    {
        for (std::size_t i = 0; i < lines.size(); ++i)
        {
            if (lines[i].find(needle) != std::string::npos)
            {
                return i;
            }
        }
        return lines.size();
    }
}

int main()
{
    Player player("Prefs", ClassCatalog::createClassByName("Runiste"));

    // Safe defaults: automation is always opt-in.
    assert(player.getAmmunitionSelectionPolicy() == "ask");
    assert(player.getMagicMaterialSelectionPolicy() == "ask");

    player.getInventory().addMaterial(MaterialCatalog::createArcaneDust(1));
    player.getInventory().addMaterial(MaterialCatalog::createRunicIronShard(1));
    player.getInventory().setPinnedItemKey(Inventory::materialItemKeyById("runic_iron_shard"), true);
    player.getInventory().setLockedItemKey(Inventory::materialItemKeyById("runic_iron_shard"), true);
    player.getInventory().setTrackedMaterialId("runic_iron_shard", true);

    const MenuScreen summary = InventoryDisplay::buildSimpleFullInventoryScreen(player);
    const auto& lines = summary.getLines();
    const std::size_t pinnedLine = findLineContaining(lines, "[ÉPINGLÉ] [VERROUILLÉ] [SUIVI] Éclat de fer runique");
    const std::size_t normalLine = findLineContaining(lines, "Poussière arcanique");
    assert(pinnedLine < lines.size());
    assert(normalLine < lines.size());
    assert(pinnedLine < normalLine);

    // Favorites are reserved for ammunition selection, not inventory ordering.
    player.getInventory().setFavoriteAmmunitionId("training_arrows", true);
    assert(player.getInventory().isFavoriteAmmunitionId("training_arrows"));
    assert(!player.getInventory().isFavoriteAmmunitionId("arcane_dust"));
    player.getInventory().setFavoriteAmmunitionId("barbed_arrows", true);
    assert(!player.getInventory().isFavoriteAmmunitionId("training_arrows"));
    assert(player.getInventory().isFavoriteAmmunitionId("barbed_arrows"));
    player.getInventory().setFavoriteAmmunitionId("training_bolts", true);
    assert(player.getInventory().isFavoriteAmmunitionId("barbed_arrows"));
    assert(player.getInventory().isFavoriteAmmunitionId("training_bolts"));

    // A true acquisition of a tracked material must be loudly highlighted.
    std::ostringstream captured;
    std::streambuf* previousBuffer = std::cout.rdbuf(captured.rdbuf());
    player.getInventory().addMaterial(MaterialCatalog::createRunicIronShard(1));
    std::cout.rdbuf(previousBuffer);
    assert(captured.str().find("OBJET SUIVI OBTENU : ÉCLAT DE FER RUNIQUE") != std::string::npos);

    // Losing one quality rank is not a new acquisition and must stay silent.
    captured.str("");
    captured.clear();
    previousBuffer = std::cout.rdbuf(captured.rdbuf());
    std::string previousQuality;
    std::string resultingQuality;
    bool dissolved = false;
    assert(player.getInventory().degradeMaterialQualityByIdAndQuality(
        "runic_iron_shard", "normal", previousQuality, resultingQuality, dissolved));
    std::cout.rdbuf(previousBuffer);
    assert(captured.str().find("OBJET SUIVI OBTENU") == std::string::npos);

    return 0;
}
