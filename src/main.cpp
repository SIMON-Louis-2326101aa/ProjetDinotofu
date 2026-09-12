// EN: main.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: main.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "core/Game.hpp"
#include "core/Console.hpp"
#include "core/VersionInfo.hpp"

#include <string>
#include <filesystem>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <linux/limits.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace
{
    // EN: Resolves the absolute path to the executable binary.
    // FR: Résout le chemin absolu vers le binaire exécutable.
    std::filesystem::path resolveExecutablePath(const char* argv0)
    {
#if defined(_WIN32)
        wchar_t buffer[MAX_PATH];
        DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        if (length > 0 && length < MAX_PATH)
        {
            return std::filesystem::path(buffer);
        }
#elif defined(__linux__)
        char buffer[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len > 0)
        {
            buffer[len] = '\0';
            return std::filesystem::path(buffer);
        }
#elif defined(__APPLE__)
        char buffer[1024];
        uint32_t size = sizeof(buffer);
        if (_NSGetExecutablePath(buffer, &size) == 0)
        {
            return std::filesystem::weakly_canonical(std::filesystem::path(buffer));
        }
#endif
        if (argv0 != nullptr && argv0[0] != '\0')
        {
            std::error_code ec;
            auto candidate = std::filesystem::weakly_canonical(std::filesystem::path(argv0), ec);
            if (!ec && !candidate.empty())
            {
                return candidate;
            }
        }
        return {};
    }

    // EN: Anchors working directory to the game root so saves, logs and assets remain self-contained.
    // FR: Ancre le répertoire de travail sur la racine du jeu pour que sauvegardes, logs et assets restent autonomes.
    void initializeGameWorkingDirectory(const char* argv0)
    {
        std::error_code ec;
        const std::filesystem::path exePath = resolveExecutablePath(argv0);
        if (exePath.empty())
        {
            return;
        }

        const std::filesystem::path exeDir = exePath.parent_path();

        // 1. If assets/ is directly next to the executable (standard release package):
        if (std::filesystem::is_directory(exeDir / "assets", ec))
        {
            std::filesystem::current_path(exeDir, ec);
            return;
        }

        // 2. If the executable is in output/ or bin/ and assets/ is in the parent directory:
        const std::filesystem::path parentDir = exeDir.parent_path();
        if (!parentDir.empty() && std::filesystem::is_directory(parentDir / "assets", ec))
        {
            std::filesystem::current_path(parentDir, ec);
            return;
        }

        // 3. Fallback: change directory to the executable's folder:
        std::filesystem::current_path(exeDir, ec);
    }
}

// EN: main declares or implements a focused behavior used by this module.
// FR: main déclare ou implémente un comportement précis utilisé par ce module.
int main(int argc, char* argv[])
{
    initializeGameWorkingDirectory(argc > 0 ? argv[0] : nullptr);

    Console::configureTerminalEncoding();

    if (argc > 1)
    {
        const std::string argument = argv[1];

        if (argument == "--version" || argument == "-v")
        {
            Console::printLine("Dinotofu V" + VersionInfo::currentVersion());
            return 0;
        }

        if (argument == "--encoding-check")
        {
            Console::printLine("Dinotofu V" + VersionInfo::currentVersion());
            Console::printLine("Contrôle encodage terminal : accents français, Épée, ténèbres, forêt, dégâts, Léthal.");
            Console::printLine("Si cette ligne est lisible, le terminal accepte correctement l'UTF-8 pour Dinotofu.");
            return 0;
        }

        if (argument == "--help" || argument == "-h")
        {
            Console::printLine("Dinotofu V" + VersionInfo::currentVersion());
            Console::printLine("Usage : ./Dinotofu [--version] [--encoding-check] [--help]");
            Console::printLine("Sans argument, le jeu se lance normalement.");
            return 0;
        }
    }

    Console::useNormalTheme();

    Game jeu;
    jeu.run();

    return 0;
}
