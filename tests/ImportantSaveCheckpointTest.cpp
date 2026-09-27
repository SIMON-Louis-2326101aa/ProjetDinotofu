#include "core/VersionInfo.hpp"
#include "entity/Player.hpp"
#include "save/SaveManager.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace
{
    std::string readText(const std::filesystem::path& path)
    {
        std::ifstream input(path);
        std::ostringstream buffer;
        buffer << input.rdbuf();
        return buffer.str();
    }
}

int main()
{
    namespace fs = std::filesystem;

    assert(VersionInfo::currentVersion() == "3.50.09");
    assert(VersionInfo::importantSaveUpdateVersion() == "3.50.09");
    assert(VersionInfo::requiresImportantSaveUpdate("3.50.08"));
    assert(VersionInfo::requiresImportantSaveUpdate("3.49.93"));
    assert(!VersionInfo::requiresImportantSaveUpdate("3.50.09"));
    assert(!VersionInfo::requiresImportantSaveUpdate("3.50.10"));

    fs::remove_all("assets/saves");

    const std::string account = "important_checkpoint_test";
    Player player;
    assert(SaveManager::saveAccountSnapshot(account));
    assert(SaveManager::savePlayerSnapshot(player, account, DifficultyMode::Normal, DeathRuleMode::NonDefinitive));

    const auto summaries = SaveManager::listPlayableCharacters(account);
    assert(!summaries.empty());

    std::string backupDirectory;
    assert(SaveManager::createImportantUpdateBackup(summaries.front(), backupDirectory));
    assert(!backupDirectory.empty());
    assert(fs::exists(backupDirectory));

    const fs::path expectedCharacterBackup = fs::path(backupDirectory)
        / (SaveManager::buildSafeFileName(summaries.front().characterName) + "__before_V3.50.09.json");
    const fs::path expectedAccountBackup = fs::path(backupDirectory) / "account__before_V3.50.09.json";
    const fs::path manifest = fs::path(backupDirectory) / "checkpoint.txt";

    assert(fs::exists(expectedCharacterBackup));
    assert(fs::exists(expectedAccountBackup));
    assert(fs::exists(manifest));

    const std::string firstBackupContent = readText(expectedCharacterBackup);
    assert(!firstBackupContent.empty());
    assert(readText(manifest).find("policy=pre_update_backup_never_overwritten") != std::string::npos);

    // A later ordinary save must not overwrite the dedicated pre-update checkpoint.
    player.gainExperience(77);
    assert(SaveManager::savePlayerSnapshot(player, account, DifficultyMode::Normal, DeathRuleMode::NonDefinitive));
    std::string secondDirectory;
    assert(SaveManager::createImportantUpdateBackup(summaries.front(), secondDirectory));
    assert(secondDirectory == backupDirectory);
    assert(readText(expectedCharacterBackup) == firstBackupContent);

    fs::remove_all("assets/saves");
    return 0;
}
