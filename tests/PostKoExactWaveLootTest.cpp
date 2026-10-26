#include <cassert>
#include <iostream>
#include <sstream>

#include "class_system/ClassCatalog.hpp"
#include "combat/EnemyCombatQueue.hpp"
#include "combat/loot/LootGenerator.hpp"
#include "core/Random.hpp"
#include "entity/Monster.hpp"
#include "entity/Player.hpp"
#include "progression/DifficultyMode.hpp"

namespace
{
    EnemyCombatQueue defeatedWave()
    {
        EnemyCombatQueue wave;
        wave.addWaitingEnemy(Monster("Bandit A", "Humain bandit", Race::Humain, 8, 60, 4, 8, 12, 0, 0));
        wave.addWaitingEnemy(Monster("Bandit B", "Humain mercenaire", Race::Humain, 12, 75, 5, 10, 15, 0, 0, false, true));
        wave.initializeFrontLine();
        while (wave.hasActiveEnemies())
        {
            wave.getActiveEnemy(0).takeDamage(99999);
            wave.removeDeadAndReplace();
        }
        return wave;
    }
}

int main()
{
    std::istringstream simulatedInput("\n\n\n\n");
    std::streambuf* previousInput = std::cin.rdbuf(simulatedInput.rdbuf());
    EnemyCombatQueue wave = defeatedWave();
    assert(wave.getDefeatedEnemyCount() == 2);

    Player carrier("Porteur", ClassCatalog::createClassByName("Guerrier"));
    carrier.getInventory().clearAll();
    Random random(123u);
    LootGenerator::givePostKoRecoveredWaveLoot(
        carrier, wave, random, DifficultyMode::Normal, 1, "exact_wave_test");

    // Capacity 1 means one exact candidate can be granted, not a whole abstract corpse bundle.
    const int objectStacks = carrier.getInventory().getWeapons().size()
        + carrier.getInventory().getArmors().size()
        + carrier.getInventory().getMaterials().size();
    const long long coinPieces = carrier.getInventory().getCoinStacks().copper
        + carrier.getInventory().getCoinStacks().iron
        + carrier.getInventory().getCoinStacks().electrum
        + carrier.getInventory().getCoinStacks().gold
        + carrier.getInventory().getCoinStacks().platinum;
    assert(objectStacks + (coinPieces > 0 ? 1 : 0) == 1);

    Player noCarrier("Sans porteur", ClassCatalog::createClassByName("Guerrier"));
    noCarrier.getInventory().clearAll();
    Random noCapacityRandom(123u);
    LootGenerator::givePostKoRecoveredWaveLoot(
        noCarrier, wave, noCapacityRandom, DifficultyMode::Normal, 0, "exact_wave_zero_test");
    assert(noCarrier.getInventory().getWeapons().empty());
    assert(noCarrier.getInventory().getArmors().empty());
    assert(noCarrier.getInventory().getMaterials().empty());
    assert(noCarrier.getInventory().getTotalCopper() == 0);
    std::cin.rdbuf(previousInput);
    return 0;
}
