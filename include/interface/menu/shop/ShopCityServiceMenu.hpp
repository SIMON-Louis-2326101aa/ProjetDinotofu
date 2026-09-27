#pragma once

#include <string>

class Player;

namespace ShopCityServiceMenu
{
    struct CityEventOfDay
    {
        std::string id;
        std::string name;
        std::string mood;
        std::string action;
    };

    inline constexpr int kCityEconomyDiscountCapPercent = 18;

    int cityRepairDaysRemaining(const Player& player);
    CityEventOfDay cityEventForAbsoluteDay(int day, const Player* player);
    bool hasRoyalBonusCityEventToday(const Player& player);

    void openCityServiceSpecialMenu(Player& player);
    void openLodgingServiceMenu(Player& player);
    void openTransportServiceMenu(Player& player);
}
