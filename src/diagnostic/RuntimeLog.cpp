// EN: RuntimeLog.cpp writes a lightweight local text log for testers.
// FR: RuntimeLog.cpp écrit un journal texte local léger pour les testeurs.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "diagnostic/RuntimeLog.hpp"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace
{
    bool runtimeLogEnabled()
    {
        const char* disabled = std::getenv("DINOTOFU_RUNTIME_LOG");
        if (disabled == nullptr)
        {
            return true;
        }

        const std::string value(disabled);
        return !(value == "0" || value == "false" || value == "FALSE" || value == "off" || value == "OFF");
    }

    std::string currentTimestamp()
    {
        const auto now = std::chrono::system_clock::now();
        const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
        std::tm localTime{};
#if defined(_WIN32)
        localtime_s(&localTime, &nowTime);
#else
        localtime_r(&nowTime, &localTime);
#endif
        std::ostringstream stream;
        stream << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
        return stream.str();
    }

    std::filesystem::path logDirectory()
    {
        return std::filesystem::path("logs");
    }

    std::filesystem::path latestLogPath()
    {
        return logDirectory() / "dinotofu_session_latest.txt";
    }

    void ensureHeaderIfNeeded(const std::filesystem::path& path)
    {
        if (std::filesystem::exists(path) && std::filesystem::file_size(path) > 0)
        {
            return;
        }

        std::ofstream file(path, std::ios::app);
        file << "DINOTOFU - journal de session beta" << '\n';
        file << "Ce fichier aide a envoyer les evenements combats/exploration au dev." << '\n';
        file << "Il ne remplace pas une sauvegarde et peut etre supprime sans risque." << '\n';
        file << "---" << '\n';
    }
}

void RuntimeLog::recordScreen(
    const std::string& title,
    const std::string& screenId,
    const std::vector<std::string>& lines
)
{
    if (!runtimeLogEnabled())
    {
        return;
    }

    try
    {
        std::filesystem::create_directories(logDirectory());
        const std::filesystem::path path = latestLogPath();
        ensureHeaderIfNeeded(path);

        std::ofstream file(path, std::ios::app);
        if (!file)
        {
            return;
        }

        file << '[' << currentTimestamp() << "] ";
        file << title << " <" << screenId << '>' << '\n';
        for (const std::string& line : lines)
        {
            file << "  - " << line << '\n';
        }
        file << '\n';
    }
    catch (...)
    {
        // Logging must never crash the game.
    }
}

std::string RuntimeLog::currentLogPath()
{
    return latestLogPath().string();
}
