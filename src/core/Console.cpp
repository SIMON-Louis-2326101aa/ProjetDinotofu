// EN: Console.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Console.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "core/Console.hpp"

#include "interface/GuiDebugExporter.hpp"

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
#include <clocale>
#include <locale>

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#endif

namespace
{

#if defined(_WIN32)
    bool isWindowsConsoleHandle(HANDLE handle)
    {
        if (handle == nullptr || handle == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        DWORD mode = 0;
        return GetConsoleMode(handle, &mode) != 0;
    }

    std::wstring utf8ToWide(const std::string& text)
    {
        if (text.empty())
        {
            return {};
        }

        int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()), nullptr, 0);
        if (size <= 0)
        {
            size = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
        }

        if (size <= 0)
        {
            return {};
        }

        std::wstring output(static_cast<std::size_t>(size), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), output.data(), size);
        return output;
    }

    std::string wideToUtf8(const std::wstring& text)
    {
        if (text.empty())
        {
            return {};
        }

        int size = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
        if (size <= 0)
        {
            return {};
        }

        std::string output(static_cast<std::size_t>(size), '\0');
        WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), output.data(), size, nullptr, nullptr);
        return output;
    }

    bool writeUtf8ToWindowsConsole(HANDLE handle, const std::string& text)
    {
        if (!isWindowsConsoleHandle(handle))
        {
            return false;
        }

        const std::wstring wideText = utf8ToWide(text);
        if (wideText.empty() && !text.empty())
        {
            return false;
        }

        std::size_t writtenTotal = 0;
        while (writtenTotal < wideText.size())
        {
            const DWORD chunkSize = static_cast<DWORD>(std::min<std::size_t>(wideText.size() - writtenTotal, 32000));
            DWORD written = 0;
            if (WriteConsoleW(handle, wideText.data() + writtenTotal, chunkSize, &written, nullptr) == 0)
            {
                return false;
            }
            writtenTotal += written;
        }

        return true;
    }

    class WindowsUtf8ConsoleBuffer final : public std::streambuf
    {
    public:
        WindowsUtf8ConsoleBuffer(HANDLE handleValue, std::streambuf* fallbackValue)
            : handle(handleValue), fallback(fallbackValue)
        {
        }

    protected:
        int overflow(int character) override
        {
            if (character == traits_type::eof())
            {
                return sync() == 0 ? traits_type::not_eof(character) : traits_type::eof();
            }

            pending.push_back(static_cast<char>(character));
            if (character == '\n' || pending.size() >= 4096)
            {
                sync();
            }

            return character;
        }

        std::streamsize xsputn(const char* text, std::streamsize count) override
        {
            if (count <= 0)
            {
                return 0;
            }

            pending.append(text, static_cast<std::size_t>(count));
            if (pending.find('\n') != std::string::npos || pending.size() >= 4096)
            {
                sync();
            }

            return count;
        }

        int sync() override
        {
            if (pending.empty())
            {
                return 0;
            }

            const std::string text = pending;
            pending.clear();

            if (!writeUtf8ToWindowsConsole(handle, text) && fallback != nullptr)
            {
                fallback->sputn(text.data(), static_cast<std::streamsize>(text.size()));
                fallback->pubsync();
            }

            return 0;
        }

    private:
        HANDLE handle = nullptr;
        std::streambuf* fallback = nullptr;
        std::string pending;
    };

    void installWindowsUtf8ConsoleBuffers()
    {
        static bool installed = false;
        if (installed)
        {
            return;
        }

        HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        HANDLE errorHandle = GetStdHandle(STD_ERROR_HANDLE);
        if (!isWindowsConsoleHandle(outputHandle) && !isWindowsConsoleHandle(errorHandle))
        {
            return;
        }

        static WindowsUtf8ConsoleBuffer outputBuffer(outputHandle, std::cout.rdbuf());
        static WindowsUtf8ConsoleBuffer errorBuffer(errorHandle, std::cerr.rdbuf());
        if (isWindowsConsoleHandle(outputHandle))
        {
            std::cout.rdbuf(&outputBuffer);
        }
        if (isWindowsConsoleHandle(errorHandle))
        {
            std::cerr.rdbuf(&errorBuffer);
        }
        installed = true;
    }

    bool readUtf8LineFromWindowsConsole(std::string& line)
    {
        HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
        if (!isWindowsConsoleHandle(inputHandle))
        {
            return false;
        }

        std::wstring wideLine;
        wchar_t buffer[256];
        while (true)
        {
            DWORD readCount = 0;
            if (ReadConsoleW(inputHandle, buffer, static_cast<DWORD>(sizeof(buffer) / sizeof(buffer[0])), &readCount, nullptr) == 0)
            {
                return false;
            }

            if (readCount == 0)
            {
                continue;
            }

            for (DWORD index = 0; index < readCount; ++index)
            {
                const wchar_t character = buffer[index];
                if (character == L'\r')
                {
                    continue;
                }
                if (character == L'\n')
                {
                    line = wideToUtf8(wideLine);
                    return true;
                }
                wideLine.push_back(character);
            }
        }
    }
#endif

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

    std::filesystem::path guiForceStopFilePath()
    {
        const char* debugDirectory = std::getenv("DINOTOFU_GUI_DEBUG_DIR");
        if (debugDirectory != nullptr && std::string(debugDirectory).empty() == false)
        {
            return std::filesystem::path(debugDirectory) / "force_stop.txt";
        }

        const std::filesystem::path inputPath = guiInputFilePath();
        if (!inputPath.empty() && inputPath.has_parent_path())
        {
            return inputPath.parent_path() / "force_stop.txt";
        }

        return {};
    }

    bool tryConsumeGuiForceStopRequest()
    {
        const std::filesystem::path forceStopPath = guiForceStopFilePath();
        if (forceStopPath.empty() || !std::filesystem::exists(forceStopPath))
        {
            return false;
        }

        try
        {
            std::filesystem::remove(forceStopPath);
        }
        catch (...)
        {
            // EN: even if cleanup fails, the GUI request must still stop the hidden backend.
            // FR: meme si le nettoyage echoue, la demande IG doit quand meme arreter le moteur cache.
        }

        return true;
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

// EN: configureTerminalEncoding declares or implements a focused behavior used by this module.
// FR: configureTerminalEncoding déclare ou implémente un comportement précis utilisé par ce module.
void Console::configureTerminalEncoding()
{
    if (std::setlocale(LC_ALL, "") == nullptr)
    {
        std::setlocale(LC_ALL, "C.UTF-8");
    }
#if defined(_WIN32)
    std::setlocale(LC_ALL, ".UTF-8");
#endif

    try
    {
        std::locale::global(std::locale(""));
        std::cout.imbue(std::locale());
        std::cin.imbue(std::locale());
    }
    catch (...)
    {
        // Some minimal Windows terminals do not expose a named UTF-8 locale.
        // The code page switch below still keeps narrow UTF-8 output usable.
    }

#if defined(_WIN32)
    // EN: Dinotofu texts are encoded in UTF-8. Windows terminals may still start
    // with an old code page, which breaks French accents. We switch the current
    // console to UTF-8 at runtime; Linux/macOS terminals are generally already UTF-8.
    // FR: les textes Dinotofu sont encodés en UTF-8. Certains terminaux Windows
    // démarrent encore avec une ancienne page de code, ce qui casse les accents.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    installWindowsUtf8ConsoleBuffers();
#endif
}

// EN: clear declares or implements a focused behavior used by this module.
// FR: clear déclare ou implémente un comportement précis utilisé par ce module.
void Console::printLine(const std::string& text)
{
    std::cout << text << std::endl;
}

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
            if (tryConsumeGuiForceStopRequest())
            {
                std::exit(0);
            }

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

#if defined(_WIN32)
    if (readUtf8LineFromWindowsConsole(line))
    {
        if (trimLeadingWhitespace)
        {
            trimLeadingWhitespaceInPlace(line);
        }
        return true;
    }
#endif

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
    // FR: si l'IG affiche un écran sans choix puis que le moteur attend Entrée,
    // on réexporte ce même écran avec un vrai bouton Continuer. Sans ça, la vue
    // peut rester bloquée sur une lecture seule, notamment après création/reprise.
    // EN: keep GUI wait screens actionable.
    GuiDebugExporter::exportContinueFromLastMenu("Valide pour continuer.");

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
