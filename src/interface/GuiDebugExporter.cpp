// EN: GuiDebugExporter.cpp keeps optional GUI debug exports away from gameplay logic.
// FR: GuiDebugExporter.cpp isole les exports de debug IG optionnels de la logique de jeu.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/GuiDebugExporter.hpp"

#include "interface/GraphicalInterface.hpp"
#include "core/VersionInfo.hpp"
#include "interface/model/GuiSchemaVersion.hpp"

#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>

bool GuiDebugExporter::isEnabled()
{
    return !outputDirectory().empty();
}

void GuiDebugExporter::exportMenu(const MenuScreen& screen, const std::string& fileStem)
{
    if (!isEnabled())
    {
        return;
    }

    writeJsonFile(fileStem, GraphicalInterface::screenToJson(screen));
    updateGuiState("menu", fileStem, &screen, nullptr);
}

void GuiDebugExporter::exportCombat(const GuiCombatStateSnapshot& snapshot, const std::string& fileStem)
{
    if (!isEnabled())
    {
        return;
    }

    writeJsonFile(fileStem, GraphicalInterface::combatSnapshotToJson(snapshot));
    updateGuiState("combat", fileStem, nullptr, &snapshot);
}

std::string GuiDebugExporter::outputDirectory()
{
    const char* value = std::getenv("DINOTOFU_GUI_DEBUG_DIR");
    if (value == nullptr)
    {
        return "";
    }

    return std::string(value);
}


std::string GuiDebugExporter::guiInputFilePath()
{
    const char* explicitPath = std::getenv("DINOTOFU_GUI_INPUT_FILE");
    if (explicitPath != nullptr && std::string(explicitPath).empty() == false)
    {
        return std::string(explicitPath);
    }

    const std::string directory = outputDirectory();
    if (!directory.empty())
    {
        return (std::filesystem::path(directory) / "pending_input.txt").string();
    }

    return "";
}

std::string GuiDebugExporter::guiInputQueueDirectoryPath()
{
    const char* explicitPath = std::getenv("DINOTOFU_GUI_INPUT_QUEUE_DIR");
    if (explicitPath != nullptr && std::string(explicitPath).empty() == false)
    {
        return std::string(explicitPath);
    }

    const std::string directory = outputDirectory();
    if (!directory.empty())
    {
        return (std::filesystem::path(directory) / "input_queue").string();
    }

    const std::string inputFile = guiInputFilePath();
    if (!inputFile.empty())
    {
        std::filesystem::path inputPath(inputFile);
        if (inputPath.has_parent_path())
        {
            return (inputPath.parent_path() / "input_queue").string();
        }
    }

    return "";
}

void GuiDebugExporter::updateGuiState(
    const std::string& kind,
    const std::string& fileStem,
    const MenuScreen* screen,
    const GuiCombatStateSnapshot* combat
)
{
    static unsigned long long sequence = 0;
    static std::string latestMenuFile = "latest_menu.json";
    static std::string latestCombatFile = "latest_combat.json";

    ++sequence;
    const std::string sanitizedStem = sanitizeFileStem(fileStem);
    const std::string fileName = sanitizedStem + ".json";

    if (kind == "menu")
    {
        latestMenuFile = fileName;
    }
    else if (kind == "combat")
    {
        latestCombatFile = fileName;
    }

    std::ostringstream json;
    json << "{\n";
    json << "  \"type\": \"gui_state\",\n";
    json << "  \"schemaVersion\": " << GuiSchemaVersion::State << ",\n";
    json << "  \"gameVersion\": \"" << GraphicalInterface::escapeJson(VersionInfo::currentVersion()) << "\",\n";
    json << "  \"sequence\": " << sequence << ",\n";
    json << "  \"lastExportKind\": \"" << GraphicalInterface::escapeJson(kind) << "\",\n";
    json << "  \"lastExportFile\": \"" << GraphicalInterface::escapeJson(fileName) << "\",\n";
    json << "  \"latestMenuFile\": \"" << GraphicalInterface::escapeJson(latestMenuFile) << "\",\n";
    json << "  \"latestCombatFile\": \"" << GraphicalInterface::escapeJson(latestCombatFile) << "\",\n";
    json << "  \"inputFile\": \"" << GraphicalInterface::escapeJson(guiInputFilePath()) << "\",\n";
    json << "  \"inputQueueDir\": \"" << GraphicalInterface::escapeJson(guiInputQueueDirectoryPath()) << "\",\n";
    json << "  \"exportedAtEpoch\": " << static_cast<long long>(std::time(nullptr)) << ",\n";
    json << "  \"activeScreenId\": \"";

    if (screen != nullptr)
    {
        json << GraphicalInterface::escapeJson(screen->getScreenId());
    }
    else if (combat != nullptr)
    {
        json << GraphicalInterface::escapeJson(combat->screenId);
    }

    json << "\",\n";
    json << "  \"activeTitle\": \"";

    if (screen != nullptr)
    {
        json << GraphicalInterface::escapeJson(screen->getTitle());
    }
    else if (combat != nullptr)
    {
        json << GraphicalInterface::escapeJson(combat->title);
    }

    json << "\",\n";
    json << "  \"activeSpecializedView\": \"";

    if (screen != nullptr)
    {
        const GuiMenuSnapshot snapshot = screen->toGuiSnapshot();
        json << GraphicalInterface::escapeJson(snapshot.specializedView);
    }

    json << "\",\n";
    json << "  \"activeFocusCardCount\": ";

    if (screen != nullptr)
    {
        const GuiMenuSnapshot snapshot = screen->toGuiSnapshot();
        json << snapshot.focusCards.size();
    }
    else
    {
        json << 0;
    }

    json << ",\n";
    json << "  \"activeInfoCardCount\": ";

    if (screen != nullptr)
    {
        const GuiMenuSnapshot snapshot = screen->toGuiSnapshot();
        json << snapshot.infoCards.size();
    }
    else
    {
        json << 0;
    }

    json << ",\n";
    json << "  \"activeItemCardCount\": ";

    if (screen != nullptr)
    {
        const GuiMenuSnapshot snapshot = screen->toGuiSnapshot();
        json << snapshot.itemCards.size();
    }
    else
    {
        json << 0;
    }

    json << ",\n";
    json << "  \"activeNoticeCardCount\": ";

    if (screen != nullptr)
    {
        const GuiMenuSnapshot snapshot = screen->toGuiSnapshot();
        json << snapshot.noticeCards.size();
    }
    else
    {
        json << 0;
    }

    json << ",\n";
    json << "  \"activeInputMode\": \"";

    if (screen != nullptr)
    {
        json << GraphicalInterface::escapeJson(screen->getInputMode());
    }

    json << "\",\n";
    json << "  \"activeInputIntent\": \"";

    if (screen != nullptr)
    {
        const GuiMenuSnapshot snapshot = screen->toGuiSnapshot();
        json << GraphicalInterface::escapeJson(snapshot.inputIntent);
    }

    json << "\",\n";
    json << "  \"activeExpectedInputPattern\": \"";

    if (screen != nullptr)
    {
        const GuiMenuSnapshot snapshot = screen->toGuiSnapshot();
        json << GraphicalInterface::escapeJson(snapshot.expectedInputPattern);
    }

    json << "\",\n";
    json << "  \"activePrimarySubmitLabel\": \"";

    if (screen != nullptr)
    {
        const GuiMenuSnapshot snapshot = screen->toGuiSnapshot();
        json << GraphicalInterface::escapeJson(snapshot.primarySubmitLabel);
    }

    json << "\",\n";
    json << "  \"activeAcceptsEmptyInput\": ";

    if (screen != nullptr)
    {
        const GuiMenuSnapshot snapshot = screen->toGuiSnapshot();
        json << (snapshot.acceptsEmptyInput ? "true" : "false");
    }
    else
    {
        json << "false";
    }

    json << ",\n";
    json << "  \"activeHasNumericRange\": ";

    if (screen != nullptr)
    {
        json << (screen->getHasNumericRange() ? "true" : "false");
    }
    else
    {
        json << "false";
    }

    json << ",\n";
    json << "  \"activeNumericMin\": ";

    if (screen != nullptr)
    {
        json << screen->getNumericMin();
    }
    else
    {
        json << 0;
    }

    json << ",\n";
    json << "  \"activeNumericMax\": ";

    if (screen != nullptr)
    {
        json << screen->getNumericMax();
    }
    else
    {
        json << 0;
    }

    json << ",\n";
    json << "  \"activeHasTextLengthRange\": ";

    if (screen != nullptr)
    {
        json << (screen->getHasTextLengthRange() ? "true" : "false");
    }
    else
    {
        json << "false";
    }

    json << ",\n";
    json << "  \"activeTextMinLength\": ";

    if (screen != nullptr)
    {
        json << screen->getTextMinLength();
    }
    else
    {
        json << 0;
    }

    json << ",\n";
    json << "  \"activeTextMaxLength\": ";

    if (screen != nullptr)
    {
        json << screen->getTextMaxLength();
    }
    else
    {
        json << 0;
    }

    json << ",\n";
    json << "  \"activeScreenCategory\": \"";

    if (screen != nullptr)
    {
        const GuiMenuSnapshot snapshot = screen->toGuiSnapshot();
        json << GraphicalInterface::escapeJson(snapshot.screenCategory);
    }
    else if (combat != nullptr)
    {
        json << "combat";
    }

    json << "\",\n";
    json << "  \"activeActionCount\": ";

    if (screen != nullptr)
    {
        const GuiMenuSnapshot snapshot = screen->toGuiSnapshot();
        json << snapshot.actionCount;
    }
    else
    {
        json << 0;
    }

    json << ",\n";
    json << "  \"activeEnabledActionCount\": ";

    if (screen != nullptr)
    {
        const GuiMenuSnapshot snapshot = screen->toGuiSnapshot();
        json << snapshot.enabledActionCount;
    }
    else
    {
        json << 0;
    }

    json << ",\n";
    json << "  \"activeDangerActionCount\": ";

    if (screen != nullptr)
    {
        const GuiMenuSnapshot snapshot = screen->toGuiSnapshot();
        json << snapshot.dangerActionCount;
    }
    else
    {
        json << 0;
    }

    json << ",\n";
    json << "  \"activeLockedActionCount\": ";

    if (screen != nullptr)
    {
        const GuiMenuSnapshot snapshot = screen->toGuiSnapshot();
        json << snapshot.lockedActionCount;
    }
    else
    {
        json << 0;
    }

    json << ",\n";
    json << "  \"activeRecommendationMode\": \"";

    if (screen != nullptr)
    {
        const GuiMenuSnapshot snapshot = screen->toGuiSnapshot();
        json << GraphicalInterface::escapeJson(snapshot.recommendationMode);
    }

    json << "\",\n";
    json << "  \"activeRecommendedChoice\": ";

    if (screen != nullptr)
    {
        const GuiMenuSnapshot snapshot = screen->toGuiSnapshot();
        json << snapshot.recommendedChoice;
    }
    else
    {
        json << -1;
    }

    json << ",\n";
    json << "  \"activeRecommendationReason\": \"";

    if (screen != nullptr)
    {
        const GuiMenuSnapshot snapshot = screen->toGuiSnapshot();
        json << GraphicalInterface::escapeJson(snapshot.recommendationReason);
    }

    json << "\",\n";
    json << "  \"activeHasKnowledgeSensitiveAction\": ";

    if (screen != nullptr)
    {
        const GuiMenuSnapshot snapshot = screen->toGuiSnapshot();
        json << (snapshot.hasKnowledgeSensitiveAction ? "true" : "false");
    }
    else
    {
        json << "false";
    }

    json << ",\n";
    json << "  \"activeHasPagination\": ";

    if (screen != nullptr)
    {
        json << (screen->getHasPagination() ? "true" : "false");
    }
    else
    {
        json << "false";
    }

    json << ",\n";
    json << "  \"activePageNumber\": ";

    if (screen != nullptr && screen->getHasPagination())
    {
        json << (screen->getPageIndex() + 1);
    }
    else
    {
        json << 0;
    }

    json << ",\n";
    json << "  \"activeTotalPages\": ";

    if (screen != nullptr && screen->getHasPagination())
    {
        json << screen->getTotalPages();
    }
    else
    {
        json << 0;
    }

    json << "\n";
    json << "}";

    writeJsonFile("gui_state", json.str());
}

std::string GuiDebugExporter::sanitizeFileStem(const std::string& fileStem)
{
    std::string sanitized;

    for (char character : fileStem)
    {
        const bool alphaNumeric =
            (character >= 'a' && character <= 'z') ||
            (character >= 'A' && character <= 'Z') ||
            (character >= '0' && character <= '9');

        if (alphaNumeric || character == '-' || character == '_')
        {
            sanitized.push_back(character);
        }
        else
        {
            sanitized.push_back('_');
        }
    }

    if (sanitized.empty())
    {
        return "gui_snapshot";
    }

    return sanitized;
}

void GuiDebugExporter::writeJsonFile(const std::string& fileStem, const std::string& jsonContent)
{
    const std::string directory = outputDirectory();
    if (directory.empty())
    {
        return;
    }

    try
    {
        std::filesystem::create_directories(directory);
        const std::filesystem::path outputPath = std::filesystem::path(directory) / (sanitizeFileStem(fileStem) + ".json");
        const std::filesystem::path temporaryPath = outputPath.string() + ".tmp";

        {
            std::ofstream file(temporaryPath, std::ios::out | std::ios::trunc);
            if (!file)
            {
                return;
            }

            file << jsonContent << std::endl;
        }

        std::filesystem::rename(temporaryPath, outputPath);
    }
    catch (...)
    {
        // EN: Debug export must never break the game.
        // FR: L'export de debug ne doit jamais casser le jeu.
    }
}
