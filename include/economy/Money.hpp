// EN: Money.hpp formats Dinotofu monetary values without changing the save format.
// FR: Money.hpp formate les valeurs monétaires de Dinotofu sans changer le format de sauvegarde.

#ifndef INCLUDE_ECONOMY_MONEY_HPP
#define INCLUDE_ECONOMY_MONEY_HPP

#include <string>
#include <vector>

struct CoinBreakdown
{
    long long platinum = 0;
    long long gold = 0;
    long long electrum = 0;
    long long iron = 0;
    long long copper = 0;
};

class Money
{
public:
    // FR: L'économie historique du jeu parlait surtout en pièces d'or.
    // Depuis V3.10.00, le portefeuille peut stocker toutes les pièces via un total en cuivre,
    // tout en gardant les anciens prix et sauvegardes en or compatibles.
    static constexpr int COPPER_PER_IRON = 10;
    static constexpr int IRON_PER_ELECTRUM = 10;
    static constexpr int ELECTRUM_PER_GOLD = 10;
    static constexpr int GOLD_PER_PLATINUM = 10;

    static constexpr long long COPPER_PER_GOLD = static_cast<long long>(COPPER_PER_IRON) * IRON_PER_ELECTRUM * ELECTRUM_PER_GOLD;
    static constexpr long long COPPER_PER_PLATINUM = COPPER_PER_GOLD * GOLD_PER_PLATINUM;

    static std::string coinScaleText();
    static std::string formatGold(int goldAmount);
    static std::string formatGoldWithRaw(int goldAmount);
    static std::string formatCopper(int copperAmount);
    static std::string formatCopper(long long copperAmount);
    static std::string formatWalletFromCopper(long long copperAmount);
    static std::string formatWalletTotalFromCopper(long long copperAmount);
    static std::string formatCurrencyOverviewFromCopper(long long copperAmount);
    static std::string formatSeparatedNumber(long long value);
    static long long copperFromGold(int goldAmount);
    static CoinBreakdown breakdownFromGold(int goldAmount);
    static CoinBreakdown breakdownFromCopper(int copperAmount);
    static CoinBreakdown breakdownFromCopper(long long copperAmount);

private:
    static std::string formatBreakdown(const CoinBreakdown& breakdown, bool includeZeroCoins = false);
};

#endif
