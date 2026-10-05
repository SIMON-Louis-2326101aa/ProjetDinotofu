#include "entity/Player.hpp"
#include "save/SaveManager.hpp"
#include "progression/DifficultyMode.hpp"
#include "progression/DeathRuleMode.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace
{
    void writeSave(
        const std::filesystem::path& path,
        const std::string& name,
        int saveVersion,
        long long totalCopper,
        const CoinBreakdown* stacks = nullptr
    )
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream output(path);
        output
            << "{\n"
            << "  \"saveVersion\": " << saveVersion << ",\n"
            << "  \"account\": \"economy_migration_test\",\n"
            << "  \"name\": \"" << name << "\",\n"
            << "  \"class\": \"Chevalier\",\n"
            << "  \"race\": \"Humain\",\n"
            << "  \"gameVersion\": \"3.50.18\",\n"
            << "  \"level\": 4,\n"
            << "  \"experience\": 25,\n"
            << "  \"hp\": 40,\n"
            << "  \"maxHp\": 50,\n"
            << "  \"gold\": " << (totalCopper / 1000) << ",\n"
            << "  \"totalCopperCurrency\": " << totalCopper;
        if (stacks != nullptr)
        {
            output
                << ",\n  \"currencyCopperCoins\": " << stacks->copper
                << ",\n  \"currencyIronCoins\": " << stacks->iron
                << ",\n  \"currencyElectrumCoins\": " << stacks->electrum
                << ",\n  \"currencyGoldCoins\": " << stacks->gold
                << ",\n  \"currencyPlatinumCoins\": " << stacks->platinum;
        }
        output << "\n}\n";
    }

    CharacterSaveSummary findSummary(const std::vector<CharacterSaveSummary>& summaries, const std::string& name)
    {
        for (const CharacterSaveSummary& summary : summaries)
        {
            if (summary.characterName == name) return summary;
        }
        assert(false && "character summary missing");
        return CharacterSaveSummary{};
    }
}

int main()
{
    namespace fs = std::filesystem;
    fs::remove_all("assets/saves");

    const std::string account = "economy_migration_test";
    assert(SaveManager::saveAccountSnapshot(account));

    // Schema 24: 50 legacy PO + 123 exact PC -> 50 PF (500 PC) + 123 PC = 623 PC.
    writeSave(SaveManager::getCharacterSavePath(account, "AncienneEconomie"), "AncienneEconomie", 24, 50123);
    // Schema 25: already normalized money must never be value-transformed again; it has no
    // physical denomination history yet, so schema 26 decomposes it once without changing value.
    writeSave(SaveManager::getCharacterSavePath(account, "EconomieActuelle"), "EconomieActuelle", 25, 50123);

    // Schema 26: denomination stacks must survive exactly as authored, including 51 PO.
    CoinBreakdown physicalStacks;
    physicalStacks.platinum = 2;
    physicalStacks.gold = 51;
    physicalStacks.electrum = 7;
    physicalStacks.iron = 13;
    physicalStacks.copper = 4;
    const long long physicalTotal = 2 * 10000LL + 51 * 1000LL + 7 * 100LL + 13 * 10LL + 4;
    writeSave(SaveManager::getCharacterSavePath(account, "PilesPhysiques"), "PilesPhysiques", 26, physicalTotal, &physicalStacks);

    const std::vector<CharacterSaveSummary> summaries = SaveManager::listPlayableCharacters(account);
    assert(summaries.size() == 3);

    DifficultyMode difficulty = DifficultyMode::Normal;
    DeathRuleMode deathRule = DeathRuleMode::NonDefinitive;

    Player legacy;
    assert(SaveManager::loadPlayerSnapshot(findSummary(summaries, "AncienneEconomie"), legacy, difficulty, deathRule));
    assert(legacy.getInventory().getTotalCopper() == 623);

    Player current;
    assert(SaveManager::loadPlayerSnapshot(findSummary(summaries, "EconomieActuelle"), current, difficulty, deathRule));
    assert(current.getInventory().getTotalCopper() == 50123);

    Player physical;
    assert(SaveManager::loadPlayerSnapshot(findSummary(summaries, "PilesPhysiques"), physical, difficulty, deathRule));
    assert(physical.getInventory().getTotalCopper() == physicalTotal);
    const CoinBreakdown loadedStacks = physical.getInventory().getCoinStacks();
    assert(loadedStacks.platinum == 2);
    assert(loadedStacks.gold == 51);
    assert(loadedStacks.electrum == 7);
    assert(loadedStacks.iron == 13);
    assert(loadedStacks.copper == 4);

    fs::remove_all("assets/saves");
    return 0;
}
