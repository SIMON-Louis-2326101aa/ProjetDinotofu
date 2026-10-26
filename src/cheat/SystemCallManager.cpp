#include "cheat/SystemCallManager.hpp"

#include "core/Console.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace
{
    std::string normalizeSystemCommand(std::string value)
    {
        value.erase(std::remove_if(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c) != 0; }), value.end());
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return value;
    }
}

void SystemCallManager::open(Player&, DifficultyMode, DeathRuleMode)
{
    bool open = true;
    while (open)
    {
        const std::string command = MessageScreen::askText(
            "SYSTEM CALL",
            "cheat.system_call.prompt",
            {},
            ">",
            "",
            true,
            0,
            96
        );
        Console::clear();

        const std::string normalized = normalizeSystemCommand(command);
        if (normalized.empty() || normalized == "exit" || normalized == "quit" || normalized == "back")
        {
            open = false;
            continue;
        }

        // Intentionally no help, no suggestions and no command enumeration.
        // Hidden generate_* commands are added one by one in their own gameplay-safe passes.
        MessageScreen::show(
            "SYSTEM CALL",
            "cheat.system_call.unknown",
            {"UNKNOWN SYSTEM COMMAND"}
        );
        Console::waitForEnter();
        Console::clear();
    }
}
