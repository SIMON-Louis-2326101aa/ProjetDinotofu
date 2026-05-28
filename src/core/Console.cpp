// EN: Console.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Console.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "core/Console.hpp"

#include <cstdlib>
#include <chrono>
#include <thread>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cctype>
#include <cstdio>
#include <ctime>

namespace
{
    bool isTruthyEnvironmentValue(const char* value)
    {
        if (value == nullptr)
        {
            return false;
        }

        std::string text(value);
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        return text == "1" || text == "true" || text == "yes" || text == "on" || text == "gui";
    }

    bool guiInputModeEnabled()
    {
        return isTruthyEnvironmentValue(std::getenv("DINOTOFU_GUI_INPUT_MODE"));
    }

    std::filesystem::path guiInputFilePath()
    {
        const char* explicitPath = std::getenv("DINOTOFU_GUI_INPUT_FILE");
        if (explicitPath != nullptr && std::string(explicitPath).empty() == false)
        {
            return std::filesystem::path(explicitPath);
        }

        const char* debugDirectory = std::getenv("DINOTOFU_GUI_DEBUG_DIR");
        if (debugDirectory != nullptr && std::string(debugDirectory).empty() == false)
        {
            return std::filesystem::path(debugDirectory) / "pending_input.txt";
        }

        return {};
    }

    std::filesystem::path guiInputQueueDirectory()
    {
        const char* explicitPath = std::getenv("DINOTOFU_GUI_INPUT_QUEUE_DIR");
        if (explicitPath != nullptr && std::string(explicitPath).empty() == false)
        {
            return std::filesystem::path(explicitPath);
        }

        const char* debugDirectory = std::getenv("DINOTOFU_GUI_DEBUG_DIR");
        if (debugDirectory != nullptr && std::string(debugDirectory).empty() == false)
        {
            return std::filesystem::path(debugDirectory) / "input_queue";
        }

        const std::filesystem::path inputPath = guiInputFilePath();
        if (!inputPath.empty() && inputPath.has_parent_path())
        {
            return inputPath.parent_path() / "input_queue";
        }

        return {};
    }

    std::filesystem::path guiConsumedInputFilePath()
    {
        const char* debugDirectory = std::getenv("DINOTOFU_GUI_DEBUG_DIR");
        if (debugDirectory != nullptr && std::string(debugDirectory).empty() == false)
        {
            return std::filesystem::path(debugDirectory) / "last_consumed_input.json";
        }

        const std::filesystem::path inputPath = guiInputFilePath();
        if (!inputPath.empty() && inputPath.has_parent_path())
        {
            return inputPath.parent_path() / "last_consumed_input.json";
        }

        return {};
    }

    std::string escapeSmallJsonString(const std::string& value)
    {
        std::string escaped;
        for (char character : value)
        {
            switch (character)
            {
                case '\\': escaped += "\\\\"; break;
                case '"': escaped += "\\\""; break;
                case '\n': escaped += "\\n"; break;
                case '\r': escaped += "\\r"; break;
                case '\t': escaped += "\\t"; break;
                default: escaped.push_back(character); break;
            }
        }
        return escaped;
    }

    void writeConsumedGuiInput(const std::string& line, const std::string& source, const std::string& metadataJson = "")
    {
        const std::filesystem::path consumedPath = guiConsumedInputFilePath();
        if (consumedPath.empty())
        {
            return;
        }

        try
        {
            std::filesystem::create_directories(consumedPath.parent_path());
            const std::filesystem::path temporaryPath = consumedPath.string() + ".tmp";
            {
                std::ofstream outputFile(temporaryPath, std::ios::out | std::ios::trunc);
                if (!outputFile)
                {
                    return;
                }

                outputFile << "{\n";
                outputFile << "  \"consumedAtEpoch\": " << static_cast<long long>(std::time(nullptr)) << ",\n";
                outputFile << "  \"source\": \"" << escapeSmallJsonString(source) << "\",\n";
                outputFile << "  \"command\": \"" << escapeSmallJsonString(line) << "\",\n";
                outputFile << "  \"display\": \"" << escapeSmallJsonString(line.empty() ? "[Entree]" : line) << "\",\n";
                outputFile << "  \"metadata\": ";

                if (!metadataJson.empty() && metadataJson.find('{') != std::string::npos)
                {
                    outputFile << metadataJson << "\n";
                }
                else
                {
                    outputFile << "null\n";
                }

                outputFile << "}\n";
            }
            std::filesystem::rename(temporaryPath, consumedPath);
        }
        catch (...)
        {
            // EN: GUI acknowledgement is diagnostic only.
            // FR: L'accuse de reception IG sert seulement au diagnostic.
        }
    }

    bool tryReadGuiInputQueueLine(std::string& line)
    {
        const std::filesystem::path queueDirectory = guiInputQueueDirectory();
        if (queueDirectory.empty() || !std::filesystem::exists(queueDirectory))
        {
            return false;
        }

        try
        {
            std::vector<std::filesystem::path> queuedFiles;
            for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(queueDirectory))
            {
                if (entry.is_regular_file() && entry.path().extension() == ".cmd")
                {
                    queuedFiles.push_back(entry.path());
                }
            }

            if (queuedFiles.empty())
            {
                return false;
            }

            std::sort(queuedFiles.begin(), queuedFiles.end());
            const std::filesystem::path commandPath = queuedFiles.front();

            {
                std::ifstream inputFile(commandPath);
                if (!inputFile)
                {
                    std::filesystem::remove(commandPath);
                    return false;
                }

                std::getline(inputFile, line);
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }
            }

            const std::filesystem::path metadataPath = commandPath.parent_path() / (commandPath.stem().string() + ".meta.json");
            std::string metadataJson;
            if (std::filesystem::exists(metadataPath))
            {
                std::ifstream metadataFile(metadataPath);
                if (metadataFile)
                {
                    std::ostringstream metadataStream;
                    metadataStream << metadataFile.rdbuf();
                    metadataJson = metadataStream.str();
                }
            }

            std::filesystem::remove(commandPath);
            std::filesystem::remove(metadataPath);
            writeConsumedGuiInput(line, "queue", metadataJson);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool tryReadGuiInputLine(std::string& line)
    {
        if (tryReadGuiInputQueueLine(line))
        {
            return true;
        }

        const std::filesystem::path inputPath = guiInputFilePath();
        if (inputPath.empty() || !std::filesystem::exists(inputPath))
        {
            return false;
        }

        try
        {
            std::ifstream inputFile(inputPath);
            if (!inputFile)
            {
                return false;
            }

            std::vector<std::string> lines;
            std::string current;
            while (std::getline(inputFile, current))
            {
                if (!current.empty() && current.back() == '\r')
                {
                    current.pop_back();
                }
                lines.push_back(current);
            }

            if (lines.empty())
            {
                return false;
            }

            line = lines.front();

            const std::filesystem::path temporaryPath = inputPath.string() + ".tmp";
            {
                std::ofstream outputFile(temporaryPath, std::ios::out | std::ios::trunc);
                if (!outputFile)
                {
                    return true;
                }

                for (std::size_t index = 1; index < lines.size(); ++index)
                {
                    outputFile << lines[index] << '\n';
                }
            }

            std::filesystem::rename(temporaryPath, inputPath);
            writeConsumedGuiInput(line, "legacy_file");
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    void trimLeadingWhitespaceInPlace(std::string& line)
    {
        line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char c) {
            return !std::isspace(c);
        }));
    }
}

// EN: clear declares or implements a focused behavior used by this module.
// FR: clear déclare ou implémente un comportement précis utilisé par ce module.
void Console::clear()
{
#if defined(_WIN32)
    system("cls");
#else
    const char* term = std::getenv("TERM");
    if (term != nullptr && std::string(term).empty() == false && std::string(term) != "dumb")
    {
        system("clear");
        return;
    }

    std::cout << std::string(60, '\n');
#endif
}

// EN: pauseSeconds declares or implements a focused behavior used by this module.
// FR: pauseSeconds déclare ou implémente un comportement précis utilisé par ce module.
void Console::pauseSeconds(int seconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

// EN: flushAvailableInputBuffer declares or implements a focused behavior used by this module.
// FR: flushAvailableInputBuffer déclare ou implémente un comportement précis utilisé par ce module.
void Console::flushAvailableInputBuffer()
{
    while (std::cin.rdbuf()->in_avail() > 0)
    {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

bool Console::readLine(std::string& line, bool trimLeadingWhitespace)
{
    if (guiInputModeEnabled())
    {
        while (true)
        {
            if (tryReadGuiInputLine(line))
            {
                if (trimLeadingWhitespace)
                {
                    trimLeadingWhitespaceInPlace(line);
                }
                return true;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(80));
        }
    }

    if (tryReadGuiInputLine(line))
    {
        if (trimLeadingWhitespace)
        {
            trimLeadingWhitespaceInPlace(line);
        }
        return true;
    }

    if (trimLeadingWhitespace)
    {
        if (!std::getline(std::cin >> std::ws, line))
        {
            return false;
        }
    }
    else if (!std::getline(std::cin, line))
    {
        return false;
    }

    if (!line.empty() && line.back() == '\r')
    {
        line.pop_back();
    }

    return true;
}

// EN: waitForEnter declares or implements a focused behavior used by this module.
// FR: waitForEnter déclare ou implémente un comportement précis utilisé par ce module.
void Console::waitForEnter()
{
    flushAvailableInputBuffer();

    std::cout << std::endl;
    std::cout << "Appuie sur Entrée pour continuer...";
    std::cout.flush();

    std::string ligne;
    readLine(ligne, false);

    flushAvailableInputBuffer();

    std::cout << std::endl;
}

// EN: askNumberBetween declares or implements a focused behavior used by this module.
// FR: askNumberBetween déclare ou implémente un comportement précis utilisé par ce module.
int Console::askNumberBetween(int min, int max, const std::string& errorMessage)
{
    while (true)
    {
        std::string ligne;

        if (!readLine(ligne, true))
        {
            std::cin.clear();
            int fallbackChoice = (min <= 0 && 0 <= max) ? 0 : min;
            std::cout << "Entrée interrompue. Choix de secours appliqué." << std::endl;
            return fallbackChoice;
        }

        std::istringstream flux(ligne);

        int choice;
        char caractereEnTrop;

        if (!(flux >> choice))
        {
            std::cout << errorMessage << std::endl;
            std::cout << "> ";
            continue;
        }

        if (flux >> caractereEnTrop)
        {
            std::cout << errorMessage << std::endl;
            std::cout << "> ";
            continue;
        }

        if (choice < min || choice > max)
        {
            std::cout << errorMessage << std::endl;
            std::cout << "> ";
            continue;
        }

        flushAvailableInputBuffer();

        return choice;
    }
}
