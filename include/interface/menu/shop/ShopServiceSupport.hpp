#pragma once

#include <string>
#include <vector>

class Player;

namespace ShopServiceSupport
{
    std::string localReputationLineForPlayer(const Player& player);

    bool askShopConfirmation(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        const std::string& confirmLabel,
        const std::string& cancelLabel,
        const std::string& actionPrefix
    );

    void showShopResult(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines
    );

    bool payServiceWithVoucherOrGold(
        Player& player,
        const std::string& voucherId,
        const std::string& voucherName,
        int fallbackPrice,
        std::vector<std::string>& resultLines
    );

    void showLocalServiceResult(
        const std::string& title,
        const std::string& screenId,
        Player& player,
        std::vector<std::string> resultLines,
        int timeUnits
    );
}
