#include "core/VersionInfo.hpp"
#include "entity/Player.hpp"
#include "save/SaveManager.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

namespace
{
    void writeText(const std::filesystem::path& path, const std::string& text)
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream output(path);
        output << text;
    }
}

int main()
{
    namespace fs = std::filesystem;
    fs::remove_all("assets/saves");

    const std::string account = "legacy_migration_test";
    assert(SaveManager::saveAccountSnapshot(account));

    // Legacy save carrying only gameVersion: createdForVersion/lastAdaptedVersion did not exist yet.
    const fs::path legacyPath = SaveManager::getCharacterSavePath(account, "Ancien");
    writeText(legacyPath,
        "{\n"
        "  \"account\": \"legacy_migration_test\",\n"
        "  \"name\": \"Ancien\",\n"
        "  \"class\": \"Chevalier\",\n"
        "  \"race\": \"Humain\",\n"
        "  \"gameVersion\": \"2.75.00\",\n"
        "  \"level\": 4,\n"
        "  \"experience\": 25,\n"
        "  \"hp\": 40,\n"
        "  \"maxHp\": 50,\n"
        "  \"gold\": 12\n"
        "}\n");

    auto summaries = SaveManager::listPlayableCharacters(account);
    assert(summaries.size() == 1);
    assert(summaries.front().lastAdaptedVersion == "2.75.00");
    assert(VersionInfo::requiresImportantSaveUpdate(summaries.front().lastAdaptedVersion));

    std::string checkpointDir;
    assert(SaveManager::createImportantUpdateBackup(summaries.front(), checkpointDir));
    assert(!checkpointDir.empty());

    Player loaded;
    DifficultyMode difficulty = DifficultyMode::Normal;
    DeathRuleMode deathRule = DeathRuleMode::NonDefinitive;
    assert(SaveManager::loadPlayerSnapshot(summaries.front(), loaded, difficulty, deathRule));
    loaded.applyHeavyVersionAdaptation(difficulty);
    assert(loaded.getLastAdaptedVersion() == VersionInfo::currentVersion());
    assert(SaveManager::savePlayerSnapshot(loaded, account, difficulty, deathRule));

    summaries = SaveManager::listPlayableCharacters(account);
    assert(summaries.size() == 1);
    assert(summaries.front().lastAdaptedVersion == VersionInfo::currentVersion());
    assert(!VersionInfo::requiresImportantSaveUpdate(summaries.front().lastAdaptedVersion));

    fs::remove_all("assets/saves");
    return 0;
}
