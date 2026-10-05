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

    const std::string currentVersion = VersionInfo::currentVersion();
    const std::string checkpointVersion = VersionInfo::importantSaveUpdateVersion();
    assert(VersionInfo::compare(currentVersion, checkpointVersion) >= 0);
    assert(checkpointVersion == "3.50.33");
    assert(VersionInfo::requiresImportantSaveUpdate("3.50.08"));
    assert(VersionInfo::requiresImportantSaveUpdate("3.49.93"));
    assert(VersionInfo::requiresImportantSaveUpdate("2.99.99"));
    assert(VersionInfo::requiresImportantSaveUpdate("1.00.00"));
    assert(VersionInfo::requiresImportantSaveUpdate("inconnue"));
    assert(VersionInfo::requiresImportantSaveUpdate(""));
    assert(VersionInfo::requiresImportantSaveUpdate("3.50.12"));
    assert(VersionInfo::requiresImportantSaveUpdate("3.50.18"));
    assert(VersionInfo::requiresImportantSaveUpdate("3.50.30"));
    assert(VersionInfo::requiresImportantSaveUpdate("3.50.32"));
    assert(!VersionInfo::requiresImportantSaveUpdate("3.50.33"));
    assert(!VersionInfo::requiresImportantSaveUpdate(currentVersion));
    assert(!VersionInfo::requiresImportantSaveUpdate("3.51.00"));

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
        / (SaveManager::buildSafeFileName(summaries.front().characterName) + "__before_V" + checkpointVersion + ".json");
    const fs::path expectedAccountBackup = fs::path(backupDirectory) / ("account__before_V" + checkpointVersion + ".json");
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
