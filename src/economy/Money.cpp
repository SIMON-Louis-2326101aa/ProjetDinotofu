// EN: Money.cpp formats Dinotofu monetary values.
// FR: Money.cpp formate les valeurs monétaires de Dinotofu.

#include "economy/Money.hpp"

#include <cstdlib>
#include <sstream>

namespace
{
    void appendCoin(std::vector<std::string>& parts, int amount, const std::string& singular, const std::string& plural)
    {
        if (amount <= 0)
        {
            return;
        }

        parts.push_back(std::to_string(amount) + " " + (amount == 1 ? singular : plural));
    }
}

std::string Money::coinScaleText()
{
    return "1 platine = 10 or | 1 or = 10 électrum | 1 électrum = 10 fer | 1 fer = 10 cuivre";
}

CoinBreakdown Money::breakdownFromGold(int goldAmount)
{
    CoinBreakdown breakdown;
    if (goldAmount < 0)
    {
        goldAmount = 0;
    }

    breakdown.platinum = goldAmount / GOLD_PER_PLATINUM;
    breakdown.gold = goldAmount % GOLD_PER_PLATINUM;
    return breakdown;
}

CoinBreakdown Money::breakdownFromCopper(int copperAmount)
{
    CoinBreakdown breakdown;
    if (copperAmount < 0)
    {
        copperAmount = 0;
    }

    const int copperPerElectrum = COPPER_PER_IRON * IRON_PER_ELECTRUM;
    const int copperPerGold = copperPerElectrum * ELECTRUM_PER_GOLD;
    const int copperPerPlatinum = copperPerGold * GOLD_PER_PLATINUM;

    breakdown.platinum = copperAmount / copperPerPlatinum;
    copperAmount %= copperPerPlatinum;
    breakdown.gold = copperAmount / copperPerGold;
    copperAmount %= copperPerGold;
    breakdown.electrum = copperAmount / copperPerElectrum;
    copperAmount %= copperPerElectrum;
    breakdown.iron = copperAmount / COPPER_PER_IRON;
    breakdown.copper = copperAmount % COPPER_PER_IRON;
    return breakdown;
}

std::string Money::formatBreakdown(const CoinBreakdown& breakdown)
{
    std::vector<std::string> parts;
    appendCoin(parts, breakdown.platinum, "platine", "platines");
    appendCoin(parts, breakdown.gold, "or", "or");
    appendCoin(parts, breakdown.electrum, "électrum", "électrum");
    appendCoin(parts, breakdown.iron, "fer", "fer");
    appendCoin(parts, breakdown.copper, "cuivre", "cuivres");

    if (parts.empty())
    {
        return "0 cuivre";
    }

    std::ostringstream out;
    for (std::size_t i = 0; i < parts.size(); ++i)
    {
        if (i > 0)
        {
            out << ", ";
        }
        out << parts[i];
    }
    return out.str();
}

std::string Money::formatGold(int goldAmount)
{
    return formatBreakdown(breakdownFromGold(goldAmount));
}

std::string Money::formatGoldWithRaw(int goldAmount)
{
    if (goldAmount < 0)
    {
        goldAmount = 0;
    }
    return formatGold(goldAmount) + " (" + std::to_string(goldAmount) + " po)";
}

std::string Money::formatCopper(int copperAmount)
{
    return formatBreakdown(breakdownFromCopper(copperAmount));
}
