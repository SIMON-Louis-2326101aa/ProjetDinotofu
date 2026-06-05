// EN: Money.hpp formats Dinotofu monetary values without changing the save format.
// FR: Money.hpp formate les valeurs monétaires de Dinotofu sans changer le format de sauvegarde.

#ifndef INCLUDE_ECONOMY_MONEY_HPP
#define INCLUDE_ECONOMY_MONEY_HPP

#include <string>
#include <vector>

struct CoinBreakdown
{
    int platinum = 0;
    int gold = 0;
    int electrum = 0;
    int iron = 0;
    int copper = 0;
};

class Money
{
public:
    // FR: Dans la V2.13.00, l'économie historique reste stockée en pièces d'or
    // pour ne pas casser les sauvegardes. Les nouvelles pièces servent d'affichage,
    // de lore et de base pour les futurs prix fins.
    static constexpr int COPPER_PER_IRON = 10;
    static constexpr int IRON_PER_ELECTRUM = 10;
    static constexpr int ELECTRUM_PER_GOLD = 10;
    static constexpr int GOLD_PER_PLATINUM = 10;

    static std::string coinScaleText();
    static std::string formatGold(int goldAmount);
    static std::string formatGoldWithRaw(int goldAmount);
    static std::string formatCopper(int copperAmount);
    static CoinBreakdown breakdownFromGold(int goldAmount);
    static CoinBreakdown breakdownFromCopper(int copperAmount);

private:
    static std::string formatBreakdown(const CoinBreakdown& breakdown);
};

#endif
