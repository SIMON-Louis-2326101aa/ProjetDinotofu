#pragma once

#include "interface/menu/common/MessageScreen.hpp"
#include <string>
#include <vector>

namespace PlayerUiSupport
{
    inline void showPlayerScreen(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        bool waitAndClear = false
    )
    {
        if (!lines.empty())
        {
            MessageScreen::show(title, screenId, lines, waitAndClear);
        }
    }
}
